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

#pragma mark - Private

/**
 *  获取模块名称
 *
 *  @return 模块名称
 */
+ (NSString *)_moduleName
{
    return NSStringFromClass([self class]);
}

/**
 *  获取Lua方法名称，需要过滤冒号后面所有内容以及带With、By、At等
 *
 *  @param name 原始方法
 *
 *  @return 方法
 */
- (NSString *)_getLuaMethodNameWithName:(NSString *)name
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

/**
 *  注册模块
 *
 *  @param state Lua状态机
 */
- (void)_regWithContext:(LSCContext *)context moduleName:(NSString *)moduleName
{
    lua_State *state = context.state;
    
    lua_getglobal(state, [moduleName UTF8String]);
    if (lua_isnil(state, -1))
    {
        //允许注册
        lua_newtable(state);
        
        NSMutableArray *filterMethodList = [NSMutableArray array];
        
        //解析属性
        id (*getterAction) (id, SEL) = (id (*) (id, SEL))objc_msgSend;
        
        unsigned int propertyCount = 0;
        objc_property_t *properyList = class_copyPropertyList(self.class, &propertyCount);
        for (const objc_property_t *p = properyList; p < properyList + propertyCount; p++)
        {
            const char *propName = property_getName(*p);
            
            //输出属性的特性描述
//            unsigned int attrCount = 0;
//            objc_property_attribute_t *attrs = property_copyAttributeList(*p, &attrCount);
//            for (const objc_property_attribute_t *a = attrs; a < attrs + attrCount; a++)
//            {
//                objc_property_attribute_t attr = *a;
//                NSLog(@"%s, %s", attr.name, attr.value);
//            }
//            free(attrs);
            
            NSString *propNameStr = [NSString stringWithUTF8String:propName];
            
            //添加Setter和getter方法
            [filterMethodList addObject:propNameStr];
            [filterMethodList addObject:[NSString stringWithFormat:@"set%@%@:",
                                         [[propNameStr substringToIndex:1] uppercaseString],
                                         [propNameStr substringFromIndex:1]]];
            
            if (![propNameStr hasPrefix:@"_"])
            {
                //为导出属性 
                id value = getterAction(self, NSSelectorFromString(propNameStr));
                LSCValue *propValue = [LSCValue objectValue:value];
                [propValue pushWithState:state];
                
                lua_setfield(state, -2, [propNameStr UTF8String]);
            }
        }
        free(properyList);
        
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
                lua_pushcclosure(state, ModuleMethodRouteHandler, 3);

                NSString *luaMethodName = [self _getLuaMethodNameWithName:methodName];
                lua_setfield(state, -2, [luaMethodName UTF8String]);
            }
        }
        free(methods);
        
        lua_setglobal(state, [moduleName UTF8String]);
    }
    else
    {
        @throw [NSException
                exceptionWithName:@"Unabled register module"
                reason:@"The module of the specified name already exists!"
                userInfo:nil];
    }
    
}

/**
 *  反注册模块
 *
 *  @param state Lua状态机
 */
- (void)_unregWithContext:(LSCContext *)context moduleName:(NSString *)moduleName
{
    lua_State *state = context.state;
    
    lua_getglobal(state, [moduleName UTF8String]);
    if (lua_istable(state, -1))
    {
        //注销模块
        lua_pushnil(state);
        lua_setglobal(state, [moduleName UTF8String]);
    }
}

@end
