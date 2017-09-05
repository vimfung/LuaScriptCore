//
//  LSCClass.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCObjectClass.h"
#import "LSCTuple.h"
#import "LSCModule_Private.h"
#import "LSCContext_Private.h"
#import "LSCValue_Private.h"
#import "LSCSession_Private.h"
#import "LSCManagedObjectProtocol.h"
#import "LSCPointer.h"
#import "LSCEngineAdapter.h"
#import <objc/runtime.h>
#import <objc/message.h>

@interface LSCObjectClass () <LSCManagedObjectProtocol>

/**
 对象标识
 */
@property (nonatomic, copy) NSString *_linkId;

@end

@implementation LSCObjectClass

+ (NSString *)version
{
    return @"1.0.0";
}

+ (NSString *)moduleName
{
    return @"Object";
}

+ (void)_regModule:(Class)module context:(LSCContext *)context
{
    if (![module isSubclassOfClass:[LSCObjectClass class]])
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCObjectClass' class!", NSStringFromClass(module)]];
        return;
    }
    
    lua_State *state = context.mainSession.state;
    NSString *name = [LSCModule _getModuleNameWithClass:module];
    
    [LSCEngineAdapter getGlobal:state name:name.UTF8String];
    if (![LSCEngineAdapter isNil:state index:-1])
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module of the specified name already exists!", name]];
        [LSCEngineAdapter pop:state count:1];
        return;
    }
    [LSCEngineAdapter pop:state count:1];
    
    Class superClass = class_getSuperclass(module);
    if (superClass != [LSCModule class])
    {
        NSString *superClassModuleName = [LSCModule _getModuleNameWithClass:superClass];
        [LSCEngineAdapter getGlobal:state name:[superClassModuleName UTF8String]];
        if ([LSCEngineAdapter isNil:state index:-1])
        {
            //如果父类还没有注册，则进行注册操作
            [context registerModuleWithClass:superClass];
        }
        [LSCEngineAdapter pop:state count:1];
    }
    
    [self _regClass:module withContext:context moduleName:name];
}

static int InstanceMethodRouteHandler(lua_State *state)
{
    int retCount = 0;
    
    //修复float类型在Invocation中会丢失问题，需要定义该结构体来提供给带float参数的方法。同时返回值处理也一样。
    typedef struct {float f;} LSCFloatStruct;
    
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    Class moduleClass = (__bridge Class)[LSCEngineAdapter toPointer:state
                                                              index:[LSCEngineAdapter upvalueIndex:2]];
    const char *methodNameCStr = [LSCEngineAdapter toString:state
                                                      index:[LSCEngineAdapter upvalueIndex:3]];
    NSString *methodName = [NSString stringWithUTF8String:methodNameCStr];
    SEL selector = NSSelectorFromString(methodName);
    
    if ([LSCEngineAdapter type:state index:1] != LUA_TUSERDATA)
    {
        NSString *errMsg = [NSString stringWithFormat:@"call %@ method error : missing self parameter, please call by instance:methodName(param)", methodName];
        [context raiseExceptionWithMessage:errMsg];
        return retCount;
    }
    
    //创建调用会话
    LSCSession *callSession = [context makeSessionWithState:state];
    NSArray *arguments = [callSession parseArguments];
    LSCObjectClass *instance = [arguments[0] toObject];

    NSMethodSignature *sign = [moduleClass instanceMethodSignatureForSelector:selector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
    
    //获取类实例对象
    if (instance)
    {
        [invocation setTarget:instance];
        [invocation setSelector:selector];
        [invocation retainArguments];
        
        Method m = class_getInstanceMethod(moduleClass, selector);
        for (int i = 2; i < method_getNumberOfArguments(m); i++)
        {
            char *argType = method_copyArgumentType(m, i);

            LSCValue *value = nil;
            if (i - 1 < arguments.count)
            {
                value = arguments[i - 1];
            }
            else
            {
                value = [LSCValue nilValue];
            }
            
            if (strcmp(argType, @encode(float)) == 0)
            {
                //浮点型数据
                LSCFloatStruct floatValue = {[value toDouble]};
                [invocation setArgument:&floatValue atIndex:i];
            }
            else if (strcmp(argType, @encode(double)) == 0)
            {
                //双精度浮点型
                double doubleValue = [value toDouble];
                [invocation setArgument:&doubleValue atIndex:i];
            }
            else if (strcmp(argType, @encode(int)) == 0
                     || strcmp(argType, @encode(unsigned int)) == 0
                     || strcmp(argType, @encode(long)) == 0
                     || strcmp(argType, @encode(unsigned long)) == 0
                     || strcmp(argType, @encode(short)) == 0
                     || strcmp(argType, @encode(unsigned short)) == 0
                     || strcmp(argType, @encode(char)) == 0
                     || strcmp(argType, @encode(unsigned char)) == 0)
            {
                //整型
                NSInteger intValue = [value toDouble];
                [invocation setArgument:&intValue atIndex:i];
            }
            else if (strcmp(argType, @encode(BOOL)) == 0)
            {
                //布尔类型
                BOOL boolValue = [value toBoolean];
                [invocation setArgument:&boolValue atIndex:i];
            }
            else if (strcmp(argType, @encode(id)) == 0)
            {
                //对象类型
                id obj = [value toObject];
                [invocation setArgument:&obj atIndex:i];
            }
            
            free(argType);
        }
        
        [invocation invoke];
        
        char *returnType = method_copyReturnType(m);
        
        LSCValue *retValue = nil;
        if (strcmp(returnType, @encode(id)) == 0)
        {
            //返回值为对象，添加__unsafe_unretained修饰用于修复ARC下retObj对象被释放问题。
            id __unsafe_unretained retObj = nil;
            [invocation getReturnValue:&retObj];
            
            retValue = [LSCValue objectValue:retObj];
        }
        else if (strcmp(returnType, @encode(int)) == 0
                 || strcmp(returnType, @encode(unsigned int)) == 0
                 || strcmp(returnType, @encode(long)) == 0
                 || strcmp(returnType, @encode(unsigned long)) == 0
                 || strcmp(returnType, @encode(short)) == 0
                 || strcmp(returnType, @encode(unsigned short)) == 0
                 || strcmp(returnType, @encode(char)) == 0
                 || strcmp(returnType, @encode(unsigned char)) == 0)
        {
            // i 整型
            // I 无符号整型
            // q 长整型
            // Q 无符号长整型
            // S 无符号短整型
            // c 字符型
            // C 无符号字符型
            
            NSInteger intValue = 0;
            [invocation getReturnValue:&intValue];
            retValue = [LSCValue integerValue:intValue];
        }
        else if (strcmp(returnType, @encode(float)) == 0)
        {
            // f 浮点型，需要将值保存到floatStruct结构中传入给方法，否则会导致数据丢失
            LSCFloatStruct floatStruct = {0};
            [invocation getReturnValue:&floatStruct];
            retValue = [LSCValue numberValue:@(floatStruct.f)];
            
        }
        else if (strcmp(returnType, @encode(double)) == 0)
        {
            // d 双精度浮点型
            double doubleValue = 0.0;
            [invocation getReturnValue:&doubleValue];
            retValue = [LSCValue numberValue:@(doubleValue)];
        }
        else if (strcmp(returnType, @encode(BOOL)) == 0)
        {
            //B 布尔类型
            BOOL boolValue = NO;
            [invocation getReturnValue:&boolValue];
            retValue = [LSCValue booleanValue:boolValue];
        }
        else
        {
            //nil
            retValue = nil;
        }
        
        free(returnType);
        
        if (retValue)
        {
            retCount = [callSession setReturnValue:retValue];
        }
        
    }
    
    return retCount;
}

/**
 *  对象销毁处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectDestroyHandler (lua_State *state)
{
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    LSCSession *session = [context makeSessionWithState:state];
    
    if ([LSCEngineAdapter getTop:state] > 0 && [LSCEngineAdapter isUserdata:state index:1])
    {
        //如果为userdata类型，则进行释放
        LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter toUserdata:state index:1];
        
        [LSCEngineAdapter pushValue:1 state:state];
        [LSCEngineAdapter getField:state index:-1 name:"destroy"];
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            [LSCEngineAdapter pushValue:1 state:state];
            [LSCEngineAdapter pCall:state nargs:1 nresults:0 errfunc:0];
        }
        else
        {
            [LSCEngineAdapter pop:state count:1];
        }
        [LSCEngineAdapter pop:state count:1];
        
        //释放内存
        CFBridgingRelease(ref -> value);
    }
    
    session = nil;
    
    return 0;
}

/**
 *  对象转换为字符串处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectToStringHandler (lua_State *state)
{
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    LSCSession *session = [context makeSessionWithState:state];
    
    //由于加入了实例的super对象，因此需要根据不同类型进行不同输出。since ver 1.3
    int type = [LSCEngineAdapter type:state index:1];
    switch (type)
    {
        case LUA_TUSERDATA:
        {
            LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter toUserdata:state index:1];
            LSCObjectClass *instance = (__bridge LSCObjectClass *)(ref -> value);
            [LSCEngineAdapter pushString:[[instance description] UTF8String] state:state];
            break;
        }
        case LUA_TTABLE:
        {
            [LSCEngineAdapter pushString:"<SuperClass Type>" state:state];
            break;
        }
        default:
        {
            [LSCEngineAdapter pushString:"<Unknown Type>" state:state];
            break;
        }
    }
    
    session = nil;
    
    return 1;
}

/**
 实例对象更新索引处理

 @param state 状态机
 @return 参数数量
 */
static int instanceNewIndexHandler (lua_State *state)
{
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    LSCSession *session = [context makeSessionWithState:state];
    
    //先找到实例对象的元表，向元表添加属性
    [LSCEngineAdapter getMetatable:state index:1];
    if ([LSCEngineAdapter isTable:state index:-1])
    {
        [LSCEngineAdapter pushValue:2 state:state];
        [LSCEngineAdapter pushValue:3 state:state];
        [LSCEngineAdapter rawSet:state index:-3];
    }
    
    session = nil;
    
    return 0;
}

/**
 *  创建对象时处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectCreateHandler (lua_State *state)
{
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    Class moduleClass = (__bridge Class)[LSCEngineAdapter toPointer:state
                                                              index:[LSCEngineAdapter upvalueIndex:2]];
    
    LSCSession *callSession = [context makeSessionWithState:state];
    
    //创建对象
    [moduleClass _constructObjectWithCallSession:callSession];

    return 1;
}

/**
 *  子类化
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int subClassHandler (lua_State *state)
{
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    Class moduleClass = (__bridge Class)[LSCEngineAdapter toPointer:state
                                                              index:[LSCEngineAdapter upvalueIndex:2]];
    
    if ([LSCEngineAdapter getTop:state] == 0)
    {
        [context raiseExceptionWithMessage:@"Miss the subclass name parameter"];
        return 0;
    }
    
    LSCSession *session = [context makeSessionWithState:state];
    
    NSString *subclassName = [NSString stringWithUTF8String:[LSCEngineAdapter checkString:state index:1]];
    Class subCls = objc_allocateClassPair(moduleClass, subclassName.UTF8String, 0);
    if (subCls != NULL)
    {
        objc_registerClassPair(subCls);
        [context registerModuleWithClass:subCls];
    }
    else
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"`%@` class has already exists.", subclassName]];
    }
    
    session = nil;
    
    return 0;
}


/**
 判断是否是该类型的子类

 @param state 状态机
 @return 参数数量
 */
static int subclassOfHandler (lua_State *state)
{
    if ([LSCEngineAdapter getTop:state] == 0)
    {
        [LSCEngineAdapter pushBoolean:NO state:state];
        return 1;
    }
    
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    Class moduleClass = (__bridge Class)[LSCEngineAdapter toPointer:state
                                                              index:[LSCEngineAdapter upvalueIndex:2]];
    
    
    LSCSession *session = [context makeSessionWithState:state];

    if ([LSCEngineAdapter type:state index:1] == LUA_TTABLE)
    {
        [LSCEngineAdapter getField:state index:1 name:"_nativeClass"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
        {
            Class checkClass = (__bridge Class)[LSCEngineAdapter toPointer:state index:-1];
            BOOL flag = [moduleClass isSubclassOfClass:checkClass];
            
            [LSCEngineAdapter pushBoolean:flag state:state];
            return 1;
        }
    }
    
    [LSCEngineAdapter pushBoolean:NO state:state];
    
    session = nil;
    
    return 1;
}


/**
 判断是否是该类型的实例对象

 @param state 状态机
 @return 参数数量
 */
static int instanceOfHandler (lua_State *state)
{
    if ([LSCEngineAdapter getTop:state] < 2)
    {
        [LSCEngineAdapter pushBoolean:NO state:state];
        return 1;
    }
    
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:1]];
    LSCSession *session = [context makeSessionWithState:state];
    
    LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter toUserdata:state index:1];
    LSCObjectClass *instance = (__bridge LSCObjectClass *)(ref -> value);
    
    if ([LSCEngineAdapter type:state index:2] == LUA_TTABLE)
    {
        [LSCEngineAdapter getField:state index:2 name:"_nativeClass"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
        {
            Class checkClass = (__bridge Class)[LSCEngineAdapter toPointer:state index:-1];
            BOOL flag = [instance isKindOfClass:checkClass];
            
            [LSCEngineAdapter pushBoolean:flag state:state];
            return 1;
        }
    }
    
    [LSCEngineAdapter pushBoolean:NO state:state];
    
    session = nil;
    
    return 1;
}

#pragma mark - LSCManagedObjectProtocol

- (NSString *)linkId
{
    if (!self._linkId)
    {
        self._linkId = [NSString stringWithFormat:@"%p", self];
    }
    
    return self._linkId;
}

- (BOOL)pushWithContext:(LSCContext *)context
{
    lua_State *state = context.currentSession.state;
    
    //不存在lua实例需要进行创建
    [LSCObjectClass _createLuaInstanceWithContext:context instance:self];
    
    //调用默认init方法
    [LSCEngineAdapter getField:state index:-1 name:"init"];
    if ([LSCEngineAdapter isFunction:state index:-1])
    {
        [LSCEngineAdapter pushValue:-2 state:state];
        [LSCEngineAdapter pCall:state nargs:1 nresults:0 errfunc:0];
    }
    else
    {
        [LSCEngineAdapter pop:state count:1];
    }

    return YES;
}

#pragma mark - Private


/**
 构造对象

 @param callSession 调用会话

 @return 类实例对象
 */
+ (instancetype)_constructObjectWithCallSession:(LSCSession *)callSession;
{
    LSCObjectClass *instance = [[self alloc] init];
    
    lua_State *state = callSession.state;
    
    [self _createLuaInstanceWithContext:callSession.context instance:instance];
    
    //通过_createLuaInstanceWithState方法后会创建实例并放入栈顶
    //调用实例对象的init方法
    [LSCEngineAdapter getField:state index:-1 name:"init"];
    if ([LSCEngineAdapter isFunction:state index:-1])
    {
        [LSCEngineAdapter pushValue:-2 state:state];
        
        //将create传入的参数传递给init方法
        //-3 代表有3个非参数值在栈中，由栈顶开始计算，分别是：实例对象，init方法，实例对象
        int paramCount = [LSCEngineAdapter getTop:state] - 3;
        for (int i = 1; i <= paramCount; i++)
        {
            [LSCEngineAdapter pushValue:i state:state];
        }
        
        [LSCEngineAdapter pCall:state nargs:paramCount + 1 nresults:0 errfunc:0];
    }
    else
    {
        [LSCEngineAdapter pop:state count:1];
    }
    
    return instance;
}


/**
 创建lua实例

 @param context 上下文对象
 @param instance 实例对象
 */
+ (void)_createLuaInstanceWithContext:(LSCContext *)context instance:(LSCObjectClass *)instance
{
    lua_State *state = context.currentSession.state;
    
    //先为实例对象在lua中创建内存
    LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter newUserdata:state size:sizeof(LSCUserdataRef)];
    //创建本地实例对象，赋予lua的内存块并进行保留引用
    ref -> value = (void *)CFBridgingRetain(instance);
    
    //创建一个临时table作为元表，用于在lua上动态添加属性或方法
    [LSCEngineAdapter newTable:state];
    
    [LSCEngineAdapter pushValue:-1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__index"];
    
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushCClosure:instanceNewIndexHandler n:1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__newindex"];
    
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushCClosure:objectDestroyHandler n:1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__gc"];
    
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushCClosure:objectToStringHandler n:1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
    
    [LSCEngineAdapter pushValue:-1 state:state];
    [LSCEngineAdapter setMetatable:state index:-3];
    
    NSString *classModuleName = [LSCModule _getModuleNameWithClass:instance.class];
    NSString *metaClsName = [self _metaClassNameWithClass:classModuleName];
    [LSCEngineAdapter getMetatable:state name:metaClsName.UTF8String];
    if ([LSCEngineAdapter isTable:state index:-1])
    {
        [LSCEngineAdapter setMetatable:state index:-2];
    }
    else
    {
        [LSCEngineAdapter pop:state count:1];
    }
    
    [LSCEngineAdapter pop:state count:1];
}

/**
 *  注册类型
 *
 *  @param cls        类型
 *  @param context    上下文对象
 *  @param moduleName 模块名称
 */
+ (void)_regClass:(Class)cls withContext:(LSCContext *)context moduleName:(NSString *)moduleName
{
    lua_State *state = context.mainSession.state;
    
    //创建类模块
    [LSCEngineAdapter newTable:state];
    
    //设置类名, since ver 1.3
    [LSCEngineAdapter pushString:moduleName.UTF8String state:state];
    [LSCEngineAdapter setField:state index:-2 name:"name"];
    
    //关联本地类型
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)(cls) state:state];
    [LSCEngineAdapter setField:state index:-2 name:"_nativeClass"];
    
    //关联索引
    [LSCEngineAdapter pushValue:-1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__index"];
    
    //写入模块标识
    [LSCEngineAdapter pushString:NativeModuleType.UTF8String state:state];
    [LSCEngineAdapter setField:state index:-2 name:NativeTypeKey.UTF8String];
    
    /**
     fixed : 由于OC中类方法存在继承关系，因此，直接导出某个类定义的类方法无法满足这种继承关系。
     例如：moduleName方法在Object中定义，但是当其子类调用时由于只能取到当前导出方法的类型(Object)，无法取到调用方法的类型(即Object的子类)，因此导致逻辑处理的异常。
     所以，该处改为导出其继承的所有类方法来满足该功能需要。
    **/
    //导出声明的类方法
    [self _exportModuleAllMethod:cls
                          module:cls
                         context:context
               filterMethodNames:@[@"moduleName", @"version"]];
    
    //添加创建对象方法
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)cls state:state];
    [LSCEngineAdapter pushCClosure:objectCreateHandler n:2 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"create"];
    
    //添加子类化对象方法
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)cls state:state];
    [LSCEngineAdapter pushCClosure:subClassHandler n:2 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"subclass"];
    
    //增加子类判断方法, since ver 1.3
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)cls state:state];
    [LSCEngineAdapter pushCClosure:subclassOfHandler n:2 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"subclassOf"];
    
    //关联父类模块, 放在所有方法导出之后进行关联，否则会触发__newindex方法
    if (cls != [LSCObjectClass class])
    {
        //存在父类，则直接设置父类为元表
        NSString *superClassModuleName = [LSCModule _getModuleNameWithClass:[cls superclass]];
        [LSCEngineAdapter getGlobal:state name:[superClassModuleName UTF8String]];
        if ([LSCEngineAdapter isTable:state index:-1])
        {
            //设置父类指向
            [LSCEngineAdapter pushValue:-1 state:state];
            [LSCEngineAdapter setField:state index:-3 name:"super"];
            
            //关联元表
            [LSCEngineAdapter setMetatable:state index:-2];
        }
        else
        {
            [LSCEngineAdapter pop:state count:1];
        }
    }
    
    [LSCEngineAdapter setGlobal:state name:moduleName.UTF8String];
    
    //---------创建实例对象元表---------------
    NSString *metaClassName = [self _metaClassNameWithClass:moduleName];
    [LSCEngineAdapter newMetatable:state name:metaClassName.UTF8String];
    
    [LSCEngineAdapter getGlobal:state name:moduleName.UTF8String];
    [LSCEngineAdapter setField:state index:-2 name:"class"];
    
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)(cls) state:state];
    [LSCEngineAdapter setField:state index:-2 name:"_nativeClass"];
    
    [LSCEngineAdapter pushValue:-1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__index"];
    
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushCClosure:objectDestroyHandler n:1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__gc"];
    
    [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
    [LSCEngineAdapter pushCClosure:objectToStringHandler n:1 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
    
    //给类元表绑定该实例元表
    [LSCEngineAdapter getGlobal:state name:moduleName.UTF8String];
    [LSCEngineAdapter pushValue:-2 state:state];
    [LSCEngineAdapter setField:state index:-2 name:"prototype"];
    [LSCEngineAdapter pop:state count:1];
    
    //注册实例方法
    NSMutableArray *filterMethodList = [NSMutableArray arrayWithObjects:
                                        @"create",
                                        @"subclass",
                                        @"context",
                                        @"setContext:",
                                        @"pushWithContext",
                                        @"objectId",
                                        nil];
    
    //解析方法
    unsigned int methodCount = 0;
    Method *methods = class_copyMethodList(cls, &methodCount);
    for (const Method *m = methods; m < methods + methodCount; m ++)
    {
        SEL selector = method_getName(*m);
        
        NSString *methodName = NSStringFromSelector(selector);
        if (![methodName hasPrefix:@"_"]
            && ![methodName hasPrefix:@"."]
            && ![methodName hasPrefix:@"init"]
            && ![filterMethodList containsObject:methodName])
        {
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)cls state:state];
            [LSCEngineAdapter pushString:methodName.UTF8String state:state];
            [LSCEngineAdapter pushCClosure:InstanceMethodRouteHandler n:3 state:state];
            
            NSString *luaMethodName = [LSCModule _getLuaMethodNameWithName:methodName];
            [LSCEngineAdapter setField:state index:-2 name:luaMethodName.UTF8String];
        }
    }
    free(methods);
    
    //关联父类
    Class superClass = NULL;
    if (cls != [LSCObjectClass class])
    {
        //设置父类
        superClass = [cls superclass];
        
        //获取父级元表
        NSString *superClassModuleName = [LSCModule _getModuleNameWithClass:superClass];
        NSString *superMetaClassName = [self _metaClassNameWithClass:superClassModuleName];
        [LSCEngineAdapter getMetatable:state name:superMetaClassName.UTF8String];
        if ([LSCEngineAdapter isTable:state index:-1])
        {
            //设置父类访问属性 since ver 1.3
            [LSCEngineAdapter pushValue:-1 state:state];
            [LSCEngineAdapter setField:state index:-3 name:"super"];
            
            //设置父类元表
            [LSCEngineAdapter setMetatable:state index:-2];
        }
        else
        {
            [LSCEngineAdapter pop:state count:1];
        }
    }
    else
    {
        //Object类需要增加一些特殊方法
        //创建instanceOf方法 since ver 1.3
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)context state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)cls state:state];
        [LSCEngineAdapter pushCClosure:instanceOfHandler n:2 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"instanceOf"];
    }
    
    [LSCEngineAdapter pop:state count:1];
}


/**
 获取元类名称

 @param moduleName 类型
 @return 元类名称
 */
+ (NSString *)_metaClassNameWithClass:(NSString *)moduleName
{
    return [NSString stringWithFormat:@"_%@_META_", moduleName];
}

@end
