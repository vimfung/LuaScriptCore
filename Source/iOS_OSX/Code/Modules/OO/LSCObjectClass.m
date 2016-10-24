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
#import "LSCObjectValue.h"
#import "LSCObjectClass_Private.h"
#import <objc/runtime.h>
#import <objc/message.h>

/**
 *  实例缓存池，主要用于与lua中的对象保持相同的生命周期而设定，创建时放入池中，当gc回收时从池中移除。
 */
static NSMutableSet *_instancePool = nil;
static NSMutableDictionary *_luaInstancePool = nil;


/**
 关联实例对象

 @param instance 本地实例对象
 @param ref      Lua中的实例对象引用
 */
static void associcateInstance(LSCObjectClass *instance, void **ref)
{
    static dispatch_once_t predicate;
    dispatch_once(&predicate, ^{
        _luaInstancePool = [NSMutableDictionary dictionary];
    });
    
    NSValue *luaRefValue = [NSValue valueWithPointer:ref];
    NSString *key = [NSString stringWithFormat:@"0x%llx", (long long)instance];
    [_luaInstancePool setObject:luaRefValue forKey:key];
}

static void** findInstanceRef(LSCObjectClass *instance)
{
    NSString *key = [NSString stringWithFormat:@"0x%llx", (long long)instance];
    NSValue *value = [_luaInstancePool objectForKey:key];
    if (value)
    {
        void **ref = NULL;
        [value getValue:&ref];
        
        return ref;
    }
    
    return nil;
}

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
    NSString *key = [NSString stringWithFormat:@"0x%llx", (long long)instance];
    [_luaInstancePool removeObjectForKey:key];
    [_instancePool removeObject:instance];
}

@implementation LSCObjectClass

- (instancetype)initWithContext:(LSCContext *)context
{
    if (self = [super init])
    {
        self.context = context;
        
        lua_State *state = self.context.state;
        
        //先为实例对象在lua中创建内存
        void **ref = (void **)lua_newuserdata(state, sizeof(LSCObjectClass **));
        
        //创建本地实例对象，赋予lua的内存块
        *ref = (__bridge void *)self;
        
        luaL_getmetatable(state, [self.class moduleName].UTF8String);
        if (lua_istable(state, -1))
        {
            lua_setmetatable(state, -2);
        }
        
        //关联对象
        associcateInstance(self, ref);
    }
    
    return self;
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

+ (void**)_findLuaRef:(LSCObjectClass *)instance
{
    return findInstanceRef(instance);
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
    
    //通过LSCModule的注册方法来导出类的静态方法。
    [LSCModule _regModule:module context:context];
    [self _regClass:module withContext:context moduleName:name];
}

static int InstanceMethodRouteHandler(lua_State *state)
{
    void **ref = (void **)lua_touserdata(state, 1);
    LSCObjectClass *instance = (__bridge LSCObjectClass *)(*ref);
    
    Class moduleClass = (__bridge Class)lua_touserdata(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];
    NSString *returnType = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(3))];
    SEL selector = NSSelectorFromString(methodName);

    NSMethodSignature *sign = [moduleClass instanceMethodSignatureForSelector:selector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
    
    //获取类实例对象
    if (instance)
    {
        [invocation setTarget:instance];
        [invocation setSelector:selector];
        
        int top = lua_gettop(state);
        for (int i = 2; i <= top; i++)
        {
            LSCValue *value = [LSCObjectValue valueWithState:state atIndex:i];
            
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
                case LSCValueTypeObject:
                {
                    id obj = [value toObject];
                    [invocation setArgument:&obj atIndex:index];
                    break;
                }
                case LSCValueTypePtr:
                {
                    [invocation setArgument:(void *)[value toPtr] atIndex:index];
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
            //返回值为对象，添加__unsafe_unretained修饰用于修复ARC下retObj对象被释放问题。
            id __unsafe_unretained retObj = nil;
            [invocation getReturnValue:&retObj];
            retValue = [LSCObjectValue objectValue:retObj];
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
            retValue = [LSCObjectValue integerValue:intValue];
        }
        else if ([returnType isEqualToString:@"f"]
                 || [returnType isEqualToString:@"d"])
        {
            // f 浮点型
            // d 双精度浮点型
            
            double doubleValue = 0.0;
            [invocation getReturnValue:&doubleValue];
            retValue = [LSCObjectValue numberValue:@(doubleValue)];
        }
        else if ([returnType isEqualToString:@"B"])
        {
            //B 布尔类型
            BOOL boolValue = 0.0;
            [invocation getReturnValue:&boolValue];
            retValue = [LSCObjectValue booleanValue:boolValue];
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
    void **ref = (void **)lua_touserdata(state, 1);
    LSCObjectClass *instance = (__bridge LSCObjectClass *)(*ref);
    removeInstance(instance);
    
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
    void **ref = (void **)lua_touserdata(state, 1);
    LSCObjectClass *instance = (__bridge LSCObjectClass *)(*ref);
    lua_pushstring(state, [[instance description] UTF8String]);
    
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
    LSCContext *context = (__bridge LSCContext *)lua_touserdata(state, lua_upvalueindex(1));
    Class moduleClass = (__bridge Class)lua_touserdata(state, lua_upvalueindex(2));

    //创建本地实例对象，赋予lua的内存块
    LSCObjectClass *instance = [[moduleClass alloc] initWithContext:context];
    putInstance(instance);

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
    LSCContext *context = (__bridge LSCContext *)lua_touserdata(state, lua_upvalueindex(1));
    Class moduleClass = (__bridge Class)lua_touserdata(state, lua_upvalueindex(2));
    
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

#pragma mark - Private

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
    
    lua_getglobal(state, moduleName.UTF8String);
    if (lua_istable(state, -1))
    {
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
       
        //创建实例对象元表
        luaL_newmetatable(state, moduleName.UTF8String);
        
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
            
            //获取父级元表
            luaL_getmetatable(state, [[superClass moduleName] UTF8String]);
            if (lua_istable(state, -1))
            {
                //设置父类元表
                lua_pushvalue(state, -1);
                lua_setmetatable(state, -2);
                
                //关联父类
                lua_setfield(state, -2, "super");
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
            
            size_t returnTypeLen = 256;
            char returnType[256];
            method_getReturnType(*m, returnType, returnTypeLen);
            
            NSString *methodName = NSStringFromSelector(selector);
            if (![methodName hasPrefix:@"_"]
                && ![methodName hasPrefix:@"."]
                && ![methodName hasPrefix:@"init"]
                && ![filterMethodList containsObject:methodName])
            {
                lua_pushlightuserdata(state, (__bridge void *)cls);
                lua_pushstring(state, [methodName UTF8String]);
                lua_pushstring(state, returnType);
                lua_pushcclosure(state, InstanceMethodRouteHandler, 3);
                
                NSString *luaMethodName = [LSCModule _getLuaMethodNameWithName:methodName];
                lua_setfield(state, -2, [luaMethodName UTF8String]);
            }
        }
        free(methods);
    }
    lua_pop(state, 1);
}

@end
