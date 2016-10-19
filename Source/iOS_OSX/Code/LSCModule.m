//
//  LSCModule.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"
#import "LSCModule_Private.h"
#import "LSCValue_Private.h"
#import "LSCContext_Private.h"
#import "lauxlib.h"
#import "lualib.h"
#import <objc/runtime.h>
#import <objc/message.h>

@implementation LSCModule

static int ModuleMethodRouteHandler(lua_State *state)
{
    LSCModule *module = (__bridge LSCModule *)lua_touserdata(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];
    NSString *returnType = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(3))];
    SEL selector = NSSelectorFromString(methodName);
    
    NSMethodSignature *sign = [module methodSignatureForSelector:selector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
    [invocation setTarget:module];
    [invocation setSelector:selector];
    
    int top = lua_gettop(state);
    for (int i = top - 1; i >= 0; i--)
    {
        LSCValue *value = [LSCValue valueWithState:state atIndex:-i - 1];
        
        int index = top - i + 1;
        
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
        //结构体和其他类型暂时认为和v一样无返回值
        retValue = nil;
    }
    
    if (retValue)
    {
        [retValue pushWithState:state];
        
        return 1;
    }
    
    return 0;
}

+ (NSString *)version
{
    return @"";
}

+ (NSString *)moduleName
{
    return NSStringFromClass([self class]);
}

#pragma mark - Private

+ (NSString *)_getLuaMethodNameWithName:(NSString *)name
{
    NSString *luaName = name;
    
    NSRange range = [luaName rangeOfString:@":"];
    if (range.location != NSNotFound)
    {
        luaName = [luaName substringToIndex:range.location];
    }
    
    range = [luaName rangeOfString:@"With"];
    if (range.location != NSNotFound)
    {
        luaName = [luaName substringToIndex:range.location];
    }
    
    range = [luaName rangeOfString:@"At"];
    if (range.location != NSNotFound)
    {
        luaName = [luaName substringToIndex:range.location];
    }
    
    range = [luaName rangeOfString:@"By"];
    if (range.location != NSNotFound)
    {
        luaName = [luaName substringToIndex:range.location];
    }
    
    return luaName;
}

+ (void)_regModule:(Class)module context:(LSCContext *)context
{
    if (![module isSubclassOfClass:[LSCModule class]])
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCModule' class!", NSStringFromClass(module)]];
        return;
    }
    
    lua_State *state = context.state;
    NSString *name = [module moduleName];
    
    lua_getglobal(state, [name UTF8String]);
    if (lua_isnil(state, -1))
    {
        //允许注册
        lua_newtable(state);
        
        //写入模块标识
        lua_pushstring(state, NativeModuleType.UTF8String);
        lua_setfield(state, -2, NativeTypeKey.UTF8String);
        
        [self _exportModuleMethod:module module:module context:context];
        
        lua_setglobal(state, [name UTF8String]);
    }
    else
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module of the specified name already exists!", name]];
        return;
    }
}

+ (void)_exportModuleMethod:(Class)thiz module:(Class)module context:(LSCContext *)context
{
    lua_State *state = context.state;
    
    NSMutableArray *filterMethodList = [NSMutableArray array];
    Class metaClass = objc_getMetaClass(NSStringFromClass(module).UTF8String);
    
    //解析方法
    unsigned int methodCount = 0;
    Method *methods = class_copyMethodList(metaClass, &methodCount);
    for (const Method *m = methods; m < methods + methodCount; m ++)
    {
        SEL selector = method_getName(*m);
        
        size_t returnTypeLen = 256;
        char returnType[256];
        method_getReturnType(*m, returnType, returnTypeLen);
        
        NSString *methodName = NSStringFromSelector(selector);
        if (![methodName hasPrefix:@"_"]
            && ![methodName hasPrefix:@"."]
            && ![filterMethodList containsObject:methodName])
        {
            NSString *luaMethodName = [self _getLuaMethodNameWithName:methodName];
            
            //判断是否已导出
            BOOL hasExists = NO;
            lua_getfield(state, -1, [luaMethodName UTF8String]);
            if (!lua_isnil(state, -1))
            {
                hasExists = YES;
            }
            lua_pop(state, 1);
            
            if (!hasExists)
            {
                lua_pushlightuserdata(state, (__bridge void *)thiz);
                lua_pushstring(state, [methodName UTF8String]);
                lua_pushstring(state, returnType);
                lua_pushcclosure(state, ModuleMethodRouteHandler, 3);

                lua_setfield(state, -2, [luaMethodName UTF8String]);
            }
        }
    }
    free(methods);
    
    if (module != [LSCModule class])
    {
        //如果模块不是LSCModule，则获取其父类继续进行方法导出
        [self _exportModuleMethod:thiz module:class_getSuperclass(module) context:context];
    }
}

+ (void)_unregModule:(Class)module context:(LSCContext *)context
{
    lua_State *state = context.state;
    
    if (![module isSubclassOfClass:[LSCModule class]])
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCModule' class!", NSStringFromClass(module)]];
        return;
    }
    
    NSString *name = [module moduleName];
    
    lua_getglobal(state, [name UTF8String]);
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, NativeTypeKey.UTF8String);
        LSCValue *value = [LSCValue valueWithState:state atIndex:-1];
        lua_pop(state, 1);
        
        if ([[value toString] isEqualToString:NativeModuleType])
        {
            //为模块类型，则进行注销
            lua_pushnil(state);
            lua_setglobal(state, [name UTF8String]);
        }
    }
}

@end
