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
    int retCount = 0;
    
    //修复float类型在Invocation中会丢失问题，需要定义该结构体来提供给带float参数的方法。同时返回值处理也一样。
    typedef struct {float f;} LSCFloatStruct;
    id obj = nil;
    
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    Class moduleClass = (__bridge Class)lua_topointer(state, lua_upvalueindex(2));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(3))];
    SEL selector = NSSelectorFromString(methodName);
    
    NSMethodSignature *sign = [moduleClass methodSignatureForSelector:selector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
    [invocation setTarget:moduleClass];
    [invocation setSelector:selector];
    [invocation retainArguments];
    
    int top = lua_gettop(state);
    Method m = class_getClassMethod(moduleClass, selector);
    for (int i = 2; i < method_getNumberOfArguments(m); i++)
    {
        char *argType = method_copyArgumentType(m, i);
//        NSLog(@"---- argType = %s", argType);
        
        LSCValue *value = nil;
        if (i - 1 <= top)
        {
            value = [LSCValue valueWithContext:context atIndex:i - 1];
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
            obj = [value toObject];
            [invocation setArgument:&obj atIndex:i];
        }
        
        free(argType);
    }

    [invocation invoke];
    
    char *returnType = method_copyReturnType(m);
    LSCValue *retValue = nil;
    
    if (strcmp(returnType, @encode(id)) == 0)
    {
        //返回值为对象
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
        //结构体和其他类型暂时认为和v一样无返回值
        retValue = nil;
    }
    free(returnType);
    
    if (retValue)
    {
        [retValue pushWithContext:context];
        
        retCount = 1;
    }
    
    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return retCount;
}

+ (NSString *)version
{
    return @"";
}

+ (NSString *)moduleName
{
    NSString *name = NSStringFromClass([self class]);
    
    //Fixed : 由于Swift中类名带有模块名称，因此需要根据.分割字符串，并取最后一部份为导出类名
    NSArray<NSString *> *nameComponents = [name componentsSeparatedByString:@"."];
    return nameComponents.lastObject;
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
        
        [self _exportModuleAllMethod:module
                              module:module
                             context:context
                   filterMethodNames:nil];
        
        lua_setglobal(state, [name UTF8String]);
    }
    else
    {
        [context raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module of the specified name already exists!", name]];
        return;
    }
}

+ (void)_exportModuleMethod:(Class)thiz
                     module:(Class)module
                    context:(LSCContext *)context
          filterMethodNames:(NSArray<NSString *> *)filterMethodNames
{
    lua_State *state = context.state;
    
    Class metaClass = objc_getMetaClass(NSStringFromClass(module).UTF8String);
    
    //解析方法
    unsigned int methodCount = 0;
    Method *methods = class_copyMethodList(metaClass, &methodCount);
    for (const Method *m = methods; m < methods + methodCount; m ++)
    {
        SEL selector = method_getName(*m);
        
        NSString *methodName = NSStringFromSelector(selector);
        if (![methodName hasPrefix:@"_"]
            && ![methodName hasPrefix:@"."]
            && ![filterMethodNames containsObject:methodName])
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
                lua_pushlightuserdata(state, (__bridge void *)context);
                lua_pushlightuserdata(state, (__bridge void *)thiz);
                lua_pushstring(state, [methodName UTF8String]);
                lua_pushcclosure(state, ModuleMethodRouteHandler, 3);
                
                lua_setfield(state, -2, [luaMethodName UTF8String]);
            }
        }
    }
    free(methods);
}

+ (void)_exportModuleAllMethod:(Class)thiz
                        module:(Class)module
                       context:(LSCContext *)context
             filterMethodNames:(NSArray<NSString *> *)filterMethodNames
{
    [self _exportModuleMethod:thiz module:module context:context filterMethodNames:nil];
    
    if (module != [LSCModule class])
    {
        //如果模块不是LSCModule，则获取其父类继续进行方法导出
        [self _exportModuleMethod:thiz module:class_getSuperclass(module) context:context filterMethodNames:nil];
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
        LSCValue *value = [LSCValue valueWithContext:context atIndex:-1];
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
