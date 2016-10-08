//
//  LSCClass.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCObjectClass.h"
#import "LSCClassInstance_Private.h"
#import "LSCModule_Private.h"
#import "LSCContext_Private.h"
#import "LSCValue_Private.h"
#import <objc/runtime.h>
#import <objc/message.h>

/**
 *  实例缓存池，主要用于与lua中的对象保持相同的生命周期而设定，创建时放入池中，当gc回收时从池中移除。
 */
static NSMutableSet *_instancePool = nil;
static __weak LSCClassInstance *_currentInstance = nil;

/**
 *  放入实例到缓存池中
 *
 *  @param instance 实例
 */
static void putInstance(LSCObjectClass *instance)
{
    static dispatch_once_t predicate;
    dispatch_once(&predicate, ^{
       
        _instancePool = [NSMutableSet set];
        
    });
    
    [_instancePool addObject:instance];
}

/**
 *  从缓存池中移除实例
 *
 *  @param instance 实例
 */
static void removeInstance(LSCObjectClass *instance)
{
    [_instancePool removeObject:instance];
}

@implementation LSCObjectClass

+ (NSString *)version
{
    return @"1.0.0";
}

- (NSString *)_instanceDescription:(LSCClassInstance *)instance
{
    return [NSString stringWithFormat:@"[%@ object]", [instance.ownerClass moduleName]];
}

- (void)_instanceInitialize:(LSCClassInstance *)instance
{
    
}

- (void)_instanceUninitialize:(LSCClassInstance *)instance
{
    
}

+ (LSCClassInstance *)currentInstance
{
    return _currentInstance;
}

+ (NSString *)moduleName
{
    if (self == [LSCObjectClass class])
    {
        return @"Object";
    }
    
    return [super moduleName];
}

- (void)_regWithContext:(LSCContext *)context moduleName:(NSString *)moduleName
{
    lua_State *state = context.state;
    
    if (self.superclass != [LSCModule class])
    {
        lua_getglobal(state, [[self.superclass moduleName] UTF8String]);
        if (lua_isnil(state, -1))
        {
            //如果父类还没有注册，则进行注册操作
            [context registerModuleWithClass:self.superclass];
        }
    }
    
    [self _regClass:self.class withContext:context moduleName:moduleName];
}

/**
 *  注册类型
 *
 *  @param cls        类型
 *  @param context    上下文对象
 *  @param moduleName 模块名称
 */
- (void)_regClass:(Class)cls withContext:(LSCContext *)context moduleName:(NSString *)moduleName
{
    lua_State *state = context.state;

    lua_newtable(state);

    lua_pushstring(state, [NSStringFromClass(cls) UTF8String]);
    lua_setfield(state, -2, "_nativeClassName");
    
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
        superClass = class_getSuperclass(cls);

        lua_getglobal(state, [[superClass moduleName] UTF8String]);
        if (lua_istable(state, -1))
        {
            //设置父类元表
            lua_setmetatable(state, -2);

            //关联父类
            lua_getglobal(state, [[superClass moduleName] UTF8String]);
            lua_setfield(state, -2, "super");
        }
    }
    else
    {
        //Object需要创建对象方法
        lua_pushcfunction(state, objectCreateHandler);
        lua_setfield(state, -2, "create");
        
        //子类化对象方法
        lua_pushcfunction(state, subClassHandler);
        lua_setfield(state, -2, "subclass");
    }
    
    NSMutableArray *filterMethodList = [NSMutableArray arrayWithObjects:@"create", @"subclass", nil];
    
    //解析方法
    unsigned int methodCount = 0;
    Method *methods = class_copyMethodList(self.class, &methodCount);
    for (const Method *m = methods; m < methods + methodCount; m ++)
    {
        SEL selector = method_getName(*m);
        
        size_t returnTypeLen = 256;
        char returnType[256];
        method_getReturnType(*m, returnType, returnTypeLen);
        
        NSString *methodName = NSStringFromSelector(selector);
        if (![methodName hasPrefix:@"_"]
            && ![methodName hasPrefix:@"."]
            && ![methodName hasPrefix:@"init"]
            && ![filterMethodList containsObject:methodName])
        {
            lua_pushlightuserdata(state, (__bridge void *)self);
            lua_pushstring(state, [methodName UTF8String]);
            lua_pushstring(state, returnType);
            lua_pushcclosure(state, InstanceMethodRouteHandler, 3);
            
            NSString *luaMethodName = [self _getLuaMethodNameWithName:methodName];
            lua_setfield(state, -2, [luaMethodName UTF8String]);
        }
    }
    free(methods);
    
    lua_setglobal(state, [moduleName UTF8String]);
}

static int InstanceMethodRouteHandler(lua_State *state)
{
    LSCModule *module = (__bridge LSCModule *)lua_touserdata(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];
    NSString *returnType = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(3))];
    SEL selector = NSSelectorFromString(methodName);
    
    NSMethodSignature *sign = [module methodSignatureForSelector:selector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
    
    //获取类实例对象
    LSCClassInstance *instance = [[LSCClassInstance alloc] initWithState:state atIndex:1];
    if (instance)
    {
        _currentInstance = instance;
        
        [invocation setTarget:instance.nativeObject];
        [invocation setSelector:selector];
        
        int top = lua_gettop(state);
        for (int i = 2; i <= top; i++)
        {
            LSCValue *value = [LSCValue valueWithState:state atIndex:i];
            
            int index = i;
            
            switch (value.valueType)
            {
                case LSCValueTypeMap:
                {
                    NSDictionary *dictValue = [value toDictionary];
                    [invocation setArgument:&dictValue atIndex:index];
                    break;
                }
                case LSCValueTypeArray:
                {
                    NSArray *arrayValue = [value toArray];
                    [invocation setArgument:&arrayValue atIndex:index];
                    break;
                }
                case LSCValueTypeData:
                {
                    NSData *dataValue = [value toData];
                    [invocation setArgument:&dataValue atIndex:index];
                    break;
                }
                case LSCValueTypeString:
                {
                    NSString *stringValue = [value toString];
                    [invocation setArgument:&stringValue atIndex:index];
                    break;
                }
                case LSCValueTypeNumber:
                {
                    double doubleValue = [value toDouble];
                    [invocation setArgument:&doubleValue atIndex:index];
                    break;
                }
                case LSCValueTypeBoolean:
                {
                    BOOL boolValue = [value toBoolean];
                    [invocation setArgument:&boolValue atIndex:index];
                    break;
                }
                case LSCValueTypeInteger:
                {
                    BOOL intValue = [value toInteger];
                    [invocation setArgument:&intValue atIndex:index];
                    break;
                }
                default:
                    break;
            }
        }
        
        [invocation invoke];
        
        LSCValue *retValue = nil;
        
        if ([returnType isEqualToString:@"@"])
        {
            //返回值为对象
            id retObj = nil;
            [invocation getReturnValue:&retObj];
            retValue = [LSCValue objectValue:retObj];
        }
        else if ([returnType isEqualToString:@"i"]
                 || [returnType isEqualToString:@"I"]
                 || [returnType isEqualToString:@"q"]
                 || [returnType isEqualToString:@"Q"]
                 || [returnType isEqualToString:@"s"]
                 || [returnType isEqualToString:@"S"]
                 || [returnType isEqualToString:@"c"]
                 || [returnType isEqualToString:@"C"])
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
        else if ([returnType isEqualToString:@"f"]
                 || [returnType isEqualToString:@"d"])
        {
            // f 浮点型
            // d 双精度浮点型
            
            double doubleValue = 0.0;
            [invocation getReturnValue:&doubleValue];
            retValue = [LSCValue numberValue:@(doubleValue)];
        }
        else if ([returnType isEqualToString:@"B"])
        {
            //B 布尔类型
            BOOL boolValue = 0.0;
            [invocation getReturnValue:&boolValue];
            retValue = [LSCValue booleanValue:boolValue];
        }
        else
        {
            //nil
            retValue = nil;
        }
        
        if (retValue)
        {
            [retValue pushWithState:state];
            return 1;
        }
        
    }
    
    return 0;
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
    LSCClassInstance *instance = [[LSCClassInstance alloc] initWithState:state atIndex:1];
    _currentInstance = instance;
    
    //调用对象的destroy方法
    [instance callMethodWithName:@"destroy" arguments:nil];
    
    [instance.nativeObject _instanceUninitialize:instance];
    removeInstance(instance.nativeObject);
    
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
    LSCClassInstance *instance = [[LSCClassInstance alloc] initWithState:state atIndex:1];
    _currentInstance = instance;
    
    lua_pushstring(state, [[instance.nativeObject _instanceDescription:instance] UTF8String]);
    
    return 1;
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
    if (lua_gettop(state) <= 0)
    {
        return 0;
    }
    
    lua_newtable(state);
    
    //获取类型名称
    lua_pushvalue(state, 1);
    lua_pushstring(state, "_nativeClassName");
    lua_gettable(state, -2);
    NSString *className = [NSString stringWithUTF8String:lua_tostring(state, -1)];
    Class cls = NSClassFromString(className);
    
    lua_settop(state, -3);
    
    lua_pushvalue(state, 1);
//    lua_getglobal(state, [[cls moduleName] UTF8String]);
    if (lua_istable(state, -1))
    {
        //设置元表指向类
        lua_setmetatable(state, -2);
        
        //创建_native对象
        LSCObjectClass *nativeObject = [[cls alloc] init];
        lua_pushlightuserdata(state, (__bridge void *)(nativeObject));
        lua_setfield(state, -2, "_nativeObject");

        //放入池中
        putInstance(nativeObject);
        
        //告诉原生层生成对象
        LSCClassInstance *instance = [[LSCClassInstance alloc] initWithState:state atIndex:-1];
        _currentInstance = instance;
        
        [instance.nativeObject _instanceInitialize:instance];
        
        return 1;
    }
    
    return 0;
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
    if (lua_gettop(state) <= 0)
    {
        return 0;
    }
    
    //获取当前类型的
    lua_pushvalue(state, 1);
    
    lua_getfield(state, -1, "_nativeObject");
    if (!lua_isnil(state, -1))
    {
        //实例对象不能调用该方法
        return 0;
    }
    
    NSString *nativeClassName = nil;
    lua_pop(state, 1);
    lua_getfield(state, -1, "_nativeClassName");
    if (lua_isstring(state, -1))
    {
        nativeClassName = [NSString stringWithUTF8String:lua_tostring(state, -1)];
    }
    lua_pop(state, 2);
    
    if (!nativeClassName)
    {
        //不存在原生类
        return 0;
    }
    
    if (lua_gettop(state) < 2 && !lua_istable(state, 2))
    {
        lua_newtable(state);
    }
    else
    {
        lua_pushvalue(state, 2);
    }
    
    lua_pushstring(state, [nativeClassName UTF8String]);
    lua_setfield(state, -2, "_nativeClassName");
    
    lua_pushvalue(state, -1);
    lua_setfield(state, -2, "__index");
    
    int subClassIndex = lua_gettop(state);
    
    //继承父级gc
    lua_pushvalue(state, 1);
    lua_getfield(state, -1, "__gc");
    lua_setfield(state, subClassIndex, "__gc");
    lua_pop(state, 1);

    //继承父级tostring
    lua_pushvalue(state, 1);
    lua_getfield(state, -1, "__tostring");
    lua_setfield(state, subClassIndex, "__tostring");
    lua_pop(state, 1);
    
    //设置父类元表
    lua_pushvalue(state, 1);
    lua_setmetatable(state, -2);
    
    //关联父类
    lua_pushvalue(state, 1);
    lua_setfield(state, -2, "super");
    
    return 1;
}

@end
