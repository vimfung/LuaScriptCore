//
//  LSCClass.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCObjectClass.h"
#import "LSCModule_Private.h"
#import "LSCContext_Private.h"
#import "LSCValue_Private.h"
#import "LSCLuaObjectPushProtocol.h"
#import "LSCPointer.h"
#import <objc/runtime.h>
#import <objc/message.h>

@interface LSCObjectClass () <LSCLuaObjectPushProtocol>

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

/**
 对象指针
 */
@property (nonatomic) LSCUserdataRef userdataRef;

@end

@implementation LSCObjectClass

+ (instancetype)createInsanceWithContext:(LSCContext *)context
{
    return [[self alloc] init];
}

+ (NSString *)version
{
    return @"1.0.0";
}

+ (NSString *)moduleName
{
    if (self == [LSCObjectClass class])
    {
        return @"Object";
    }
    
    return [super moduleName];
}

+ (void)_regModule:(Class)module context:(LSCContext *)context
{
    if (![module isSubclassOfClass:[LSCObjectClass class]])
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCObjectClass' class!", NSStringFromClass(module)]];
        return;
    }
    
    lua_State *state = context.state;
    NSString *name = [module moduleName];
    
    lua_getglobal(state, name.UTF8String);
    if (!lua_isnil(state, -1))
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module of the specified name already exists!", name]];
        lua_pop(state, 1);
        return;
    }
    lua_pop(state, 1);
    
    Class superClass = class_getSuperclass(module);
    if (superClass != [LSCModule class])
    {
        lua_getglobal(state, [[superClass moduleName] UTF8String]);
        if (lua_isnil(state, -1))
        {
            //如果父类还没有注册，则进行注册操作
            [context registerModuleWithClass:superClass];
        }
        lua_pop(state, 1);
    }
    
    [self _regClass:module withContext:context moduleName:name];
}

static int InstanceMethodRouteHandler(lua_State *state)
{
    int retCount = 0;
    
    //修复float类型在Invocation中会丢失问题，需要定义该结构体来提供给带float参数的方法。同时返回值处理也一样。
    typedef struct {float f;} LSCFloatStruct;
    
    LSCUserdataRef ref = (LSCUserdataRef)lua_touserdata(state, 1);
    LSCObjectClass *instance = (__bridge LSCObjectClass *)(ref -> value);
    
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    Class moduleClass = (__bridge Class)lua_topointer(state, lua_upvalueindex(2));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(3))];
    SEL selector = NSSelectorFromString(methodName);

    NSMethodSignature *sign = [moduleClass instanceMethodSignatureForSelector:selector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
    
    //获取类实例对象
    if (instance)
    {
        [invocation setTarget:instance];
        [invocation setSelector:selector];
        [invocation retainArguments];
        
        int top = lua_gettop(state);
        
        Method m = class_getInstanceMethod(moduleClass, selector);
        for (int i = 2; i < method_getNumberOfArguments(m); i++)
        {
            char *argType = method_copyArgumentType(m, i);

            LSCValue *value = nil;
            if (i <= top)
            {
                value = [LSCValue valueWithContext:context atIndex:i];
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
            [retValue pushWithContext:context];
            retCount = 1;
        }
        
    }
    
    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
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
    LSCUserdataRef ref = (LSCUserdataRef)lua_touserdata(state, 1);
    LSCObjectClass *instance = (__bridge LSCObjectClass *)ref -> value;
    instance.userdataRef = NULL;
    
    lua_pushvalue(state, 1);
    lua_getfield(state, -1, "destroy");
    if (lua_isfunction(state, -1))
    {
        lua_pushvalue(state, 1);
        lua_pcall(state, 1, 0, 0);
    }
    lua_pop(state, 2);
    
    //释放内存
    CFBridgingRelease(ref -> value);
    
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
    LSCUserdataRef ref = (LSCUserdataRef)lua_touserdata(state, 1);
    LSCObjectClass *instance = (__bridge LSCObjectClass *)(ref -> value);
    lua_pushstring(state, [[instance description] UTF8String]);
    
    return 1;
}

/**
 *  对象更新索引处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectNewIndexHandler (lua_State *state)
{
    //限于当前无法判断所定义的方法是使用.或:定义，因此对添加的属性或者方法统一添加到类表和实例元表中。
    const char *key = luaL_checkstring(state, 2);
    lua_pushvalue(state, 2);
    lua_pushvalue(state, 3);
    lua_rawset(state, 1);
    
    //查找实例元表进行添加
    lua_getfield(state, 1, "_nativeClass");
    Class moduleClass = (__bridge Class)lua_topointer(state, -1);
    
    luaL_getmetatable(state, [moduleClass moduleName].UTF8String);
    if (lua_istable(state, -1))
    {
        lua_pushvalue(state, 3);
        lua_setfield(state, -2, key);
    }
    lua_pop(state, 1);
    
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
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    Class moduleClass = (__bridge Class)lua_topointer(state, lua_upvalueindex(2));
    
    //创建对象
    [moduleClass _constructObjectWithContext:context];

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
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    Class moduleClass = (__bridge Class)lua_topointer(state, lua_upvalueindex(2));
    
    if (lua_gettop(state) == 0)
    {
        [context raiseExceptionWithMessage:@"Miss the subclass name parameter"];
        return 0;
    }
    
    NSString *subclassName = [NSString stringWithUTF8String:luaL_checkstring(state, 1)];
    Class subCls = objc_allocateClassPair(moduleClass, subclassName.UTF8String, 0);
    objc_registerClassPair(subCls);
    
    [context registerModuleWithClass:subCls];
    
    return 0;
}

#pragma mark - LSCLuaObjectPushProtocol

- (void)pushWithContext:(LSCContext *)context
{
    lua_State *state = context.state;

    if (self.userdataRef == NULL)
    {
        //创建Lua实例引用
        LSCUserdataRef ref = (LSCUserdataRef)lua_newuserdata(state, sizeof(LSCUserdataRef));
        //创建本地实例对象，赋予lua的内存块并进行保留引用
        ref -> value = (void *)CFBridgingRetain(self);
        self.userdataRef = ref;
    }
    else
    {
        lua_pushlightuserdata(state, (void *)(self.userdataRef));
    }
    
    //直接原指针返回并不等于原始变量，因此需要重新绑定元表
    luaL_getmetatable(state, [[self class] moduleName].UTF8String);
    if (lua_istable(state, -1))
    {
        lua_setmetatable(state, -2);
    }

}

#pragma mark - Private


/**
 构造对象

 @param context 上下文对象

 @return 类实例对象
 */
+ (instancetype)_constructObjectWithContext:(LSCContext *)context;
{
    LSCObjectClass *instance = [[self alloc] init];
    instance.context = context;
    
    lua_State *state = context.state;
    
    //先为实例对象在lua中创建内存
    LSCUserdataRef ref = (LSCUserdataRef)lua_newuserdata(state, sizeof(LSCUserdataRef));
    //创建本地实例对象，赋予lua的内存块并进行保留引用
    ref -> value = (void *)CFBridgingRetain(instance);
    instance.userdataRef = ref;
    
    luaL_getmetatable(state, [instance.class moduleName].UTF8String);
    if (lua_istable(state, -1))
    {
        lua_setmetatable(state, -2);
    }
    
    //调用实例对象的init方法
    lua_pushvalue(state, 1);
    lua_getfield(state, -1, "init");
    if (lua_isfunction(state, -1))
    {
        lua_pushvalue(state, 1);
        lua_pcall(state, 1, 0, 0);
        lua_pop(state, 1);
    }
    else
    {
        lua_pop(state, 2);
    }
    
    return instance;
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
    lua_State *state = context.state;
    
    //创建类模块
    lua_newtable(state);
    
    //关联本地类型
    lua_pushlightuserdata(state, (__bridge void *)(cls));
    lua_setfield(state, -2, "_nativeClass");
    
    //关联索引
    lua_pushvalue(state, -1);
    lua_setfield(state, -2, "__index");
    
    //关联更新索引处理
    lua_pushlightuserdata(state, (__bridge void *)context);
    lua_pushcclosure(state, objectNewIndexHandler, 1);
    lua_setfield(state, -2, "__newindex");
    
    //写入模块标识
    lua_pushstring(state, NativeModuleType.UTF8String);
    lua_setfield(state, -2, NativeTypeKey.UTF8String);
    
    //导出声明的类方法
    [self _exportModuleMethod:cls
                       module:cls
                      context:context
            filterMethodNames:@[@"createInsanceWithContext:"]];
    
    //添加创建对象方法
    lua_pushlightuserdata(state, (__bridge void *)context);
    lua_pushlightuserdata(state, (__bridge void *)cls);
    lua_pushcclosure(state, objectCreateHandler, 2);
    lua_setfield(state, -2, "create");
    
    //添加子类化对象方法
    lua_pushlightuserdata(state, (__bridge void *)context);
    lua_pushlightuserdata(state, (__bridge void *)cls);
    lua_pushcclosure(state, subClassHandler, 2);
    lua_setfield(state, -2, "subclass");
    
    //关联父类模块, 放在所有方法导出之后进行关联，否则会触发__newindex方法
    if (cls != [LSCObjectClass class])
    {
        //存在父类，则直接设置父类为元表
        lua_getglobal(state, [[[cls superclass] moduleName] UTF8String]);
        if (lua_istable(state, -1))
        {
            //设置父类指向
            lua_pushvalue(state, -1);
            lua_setfield(state, -3, "super");
            
            //关联元表
            lua_setmetatable(state, -2);
        }
    }
    else
    {
        //为根类，则创建一个table作为元表
        lua_newtable(state);
        
        //关联更新索引处理
        lua_pushlightuserdata(state, (__bridge void *)context);
        lua_pushlightuserdata(state, (__bridge void *)cls);
        lua_pushcclosure(state, objectNewIndexHandler, 2);
        lua_setfield(state, -2, "__newindex");
        
        lua_setmetatable(state, -2);
    }
    
    lua_setglobal(state, [moduleName UTF8String]);
    
    //创建实例对象元表
    luaL_newmetatable(state, moduleName.UTF8String);
    
    lua_pushlightuserdata(state, (__bridge void *)(cls));
    lua_setfield(state, -2, "_nativeClass");
    
    lua_pushvalue(state, -1);
    lua_setfield(state, -2, "__index");
    
    lua_pushcfunction(state, objectDestroyHandler);
    lua_setfield(state, -2, "__gc");
    
    lua_pushcfunction(state, objectToStringHandler);
    lua_setfield(state, -2, "__tostring");
    
    Class superClass = NULL;
    if (cls != [LSCObjectClass class])
    {
        //设置父类
        superClass = [cls superclass];
        
        //获取父级元表
        luaL_getmetatable(state, [[superClass moduleName] UTF8String]);
        if (lua_istable(state, -1))
        {
            //设置父类元表
            lua_setmetatable(state, -2);
        }
        else
        {
            lua_pop(state, 1);
        }
    }
    
    NSMutableArray *filterMethodList = [NSMutableArray arrayWithObjects:
                                        @"create",
                                        @"subclass",
                                        @"context",
                                        @"setContext:",
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
            lua_pushlightuserdata(state, (__bridge void *)context);
            lua_pushlightuserdata(state, (__bridge void *)cls);
            lua_pushstring(state, [methodName UTF8String]);
            lua_pushcclosure(state, InstanceMethodRouteHandler, 3);
            
            NSString *luaMethodName = [LSCModule _getLuaMethodNameWithName:methodName];
            lua_setfield(state, -2, [luaMethodName UTF8String]);
        }
    }
    free(methods);
}

@end
