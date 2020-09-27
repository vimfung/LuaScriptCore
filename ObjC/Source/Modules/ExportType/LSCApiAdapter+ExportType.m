//
//  LSCApiAdapter+ExportType.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/2.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCApiAdapter+ExportType.h"
#import "LSCContext+Private.h"
#import "LSCContext+ExportType.h"
#import "LSCFunctionValue.h"
#import "LSCValue.h"
#import "LSCTypeValue.h"
#import "LSCStringValue.h"
#import "LSCTupleValue.h"
#import "LSCInstance+Private.h"
#import "LSCTypeDescription+Private.h"
#import "LSCMainState.h"
#import "LSCExportTypeRule.h"
#import "LSCInstanceValue.h"
#import "lauxlib.h"

@implementation LSCApiAdapter (ExportType)

- (LSCTypeDescription *)getTypeDescriptionWithStackIndex:(int)stackIndex
                                                 context:(nonnull LSCContext *)context
{
    LSCTypeDescription *typeDesc = nil;
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    if (lua_type(rawState, stackIndex) == LSCBasicTypeUserdata)
    {
        LSCUserdataRef userdata = (LSCUserdataRef)lua_topointer(rawState, stackIndex);
        id value = (__bridge LSCTypeDescription *)userdata -> value;
        if ([value isKindOfClass:[LSCTypeDescription class]])
        {
            typeDesc = value;
        }
    }
    
    [mainState unlock];
    
    return typeDesc;
}

- (void)pushType:(LSCTypeDescription *)typeDescription
         context:(LSCContext *)context
{
    LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    if (typeDescription.parentTypeDescription)
    {
        //调用获取父类操作，触发父类导出
        lua_getglobal(rawState, typeDescription.parentTypeDescription.typeName.UTF8String);
        lua_pop(rawState, 1);
    }
    
    //创建类模块
    LSCUserdataRef ref = (LSCUserdataRef)lua_newuserdata(rawState, sizeof(LSCUserdataRef));
    ref -> value = (__bridge_retained void *)typeDescription;
    
    //设置类型元表
    lua_newtable(rawState);

    //设置类名, since ver 1.3
    lua_pushstring(rawState, typeDescription.typeName.UTF8String);
    lua_setfield(rawState, -2, "name");

    //构造函数
    lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
    lua_pushcclosure(rawState, objectCreateHandler, 1);
    lua_setfield(rawState, -2, "__call");
    
    //导出声明的类方法
    [typeDescription.classMethods enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, NSMutableArray<LSCMethodDescription *> * _Nonnull obj, BOOL * _Nonnull stop) {

        lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
        lua_pushstring(rawState, key.UTF8String);
        lua_pushcclosure(rawState, classMethodRouteHandler, 2);
        
        lua_setfield(rawState, -2, key.UTF8String);
        
    }];
    
    //关联索引
    lua_pushvalue(rawState, -1);
    lua_setfield(rawState, -2, "__index");
    
    //类型描述
    lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
    lua_pushcclosure(rawState, classToStringHandler, 1);
    lua_setfield(rawState, -2, "__tostring");
    
    ///添加回收方法
    lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
    lua_pushcclosure(rawState, classDestroyHandler, 1);
    lua_setfield(rawState, -2, "__gc");
    
    ///添加动态添加方法
    lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
    lua_pushcclosure(rawState, classNewIndexHandler, 1);
    lua_setfield(rawState, -2, "__newindex");

    //获取父类型
    LSCTypeDescription *parentTypeDescriptor = typeDescription.parentTypeDescription;
    if (parentTypeDescriptor)
    {
        //存在父类，则直接设置父类为元表
        lua_getglobal(rawState, parentTypeDescriptor.typeName.UTF8String);
        if (lua_type(rawState, -1) == LSCBasicTypeUserdata)
        {
            lua_pushvalue(rawState, -1);
            lua_setfield(rawState, -3, "super");
            
            //关联元表
            lua_getmetatable(rawState, -1);
            lua_setmetatable(rawState, -3);
        }
        
        //pop parent type
        lua_pop(rawState, 1);
    }
    else
    {
        //Object类型
        //添加子类化对象方法
        lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
        lua_pushcclosure(rawState, subClassHandler, 1);
        lua_setfield(rawState, -2, "subclass");
        
        //增加子类判断方法
        lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
        lua_pushcclosure(rawState, subclassOfHandler, 1);
        lua_setfield(rawState, -2, "subclassOf");
        
        //类型别名
        lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
        lua_pushcclosure(rawState, aliasHandler, 1);
        lua_setfield(rawState, -2, "alias");
    }
    
    lua_setmetatable(rawState, -2);
    
    lua_pushvalue(rawState, -1);
    lua_setglobal(rawState, typeDescription.typeName.UTF8String);

    //---------创建实例对象原型表---------------
    lua_newuserdata(rawState, sizeof(LSCUserdataRef));
    
    luaL_newmetatable(rawState, typeDescription.prototypeName.UTF8String);

    lua_pushvalue(rawState, -3);
    lua_setfield(rawState, -2, "class");

    lua_pushvalue(rawState, -1);
    lua_setfield(rawState, -2, "__index");

    //增加__newindex元方法监听，主要用于原型中注册属性
    lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
    lua_pushcclosure(rawState, prototypeNewIndexHandler, 1);
    lua_setfield(rawState, -2, "__newindex");

    lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
    lua_pushcclosure(rawState, prototypeToStringHandler, 1);
    lua_setfield(rawState, -2, "__tostring");

    //给类元表绑定该实例元表
    lua_pushvalue(rawState, -3);    //push typeDescription
    lua_getmetatable(rawState, -1); //push typeDescription metatable
    lua_remove(rawState, -2);
    
    lua_pushvalue(rawState, -3);    //push prototype
    lua_setfield(rawState, -2, "prototype");
    lua_pop(rawState, 1);

    //导出声明的实例方法
    [typeDescription.instanceMethods enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, NSMutableArray<LSCMethodDescription *> * _Nonnull obj, BOOL * _Nonnull stop) {
        
        if (![key isEqualToString:rule.constructMethodName])
        {
            //导出非构造方法
            lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
            lua_pushstring(rawState, key.UTF8String);
            lua_pushcclosure(rawState, instanceMethodRouteHandler, 2);
            
            lua_setfield(rawState, -2, key.UTF8String);
        }
        
    }];
    
    if (parentTypeDescriptor)
    {
        //关联父类
        luaL_getmetatable(rawState, parentTypeDescriptor.prototypeName.UTF8String);
        if (lua_istable(rawState, -1))
        {
            //设置父类prototype元表
            lua_setmetatable(rawState, -2);
        }
        else
        {
            lua_pop(rawState, 1);
        }
    }
    else
    {
        //Object类需要增加一些特殊方法
        //创建instanceOf方法 since ver 1.3
        lua_pushlightuserdata(rawState, (__bridge void *)typeDescription);
        lua_pushcclosure(rawState, instanceOfHandler, 1);
        lua_setfield(rawState, -2, "instanceOf");
    }
    
    lua_setmetatable(rawState, -2);
    
    //pop prototype
    lua_pop(rawState, 1);
    
    [mainState unlock];
}

- (void)pushInstance:(LSCInstance *)instance
             context:(LSCContext *)context
{
    [self pushLuaObjectWithId:instance.instanceId context:context];
}

- (void)watchGlobalWithContext:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    //为_G设置元表，用于监听其对象的获取，从而找出哪些是导出类型
    lua_State *rawState = mainState.currentState.rawState;
    lua_getglobal(rawState, "_G");
    
    if (!lua_istable(rawState, -1))
    {
        id<LSCValueType> msgValue = [LSCValue createValue:@"Invalid '_G' object."];
        [context.exceptionFunction invokeWithArguments:@[msgValue] context:context];
        lua_pop(rawState, 1);
        return;
    }

    
    //创建_G元表
    lua_newtable(rawState);
    
    //监听__index元方法
    lua_pushcclosure(rawState, globalIndexMetaMethodHandler, 0);
    lua_setfield(rawState, -2, "__index");
    
    //绑定为_G元表
    lua_setmetatable(rawState, -2);
    
    lua_pop(rawState, 1);
    
    [mainState unlock];
}

static int globalIndexMetaMethodHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        //获取key
        id<LSCValueType> value = [LSCValue createValueWithContext:context stackIndex:2];
        NSString *key = value.rawValue;
        
        lua_rawget(rawState, 1);
        if (lua_isnil(rawState, -1))
        {
            //检测key对应的是否为导出类型
            LSCTypeDescription *typeDescriptor = [context typeDescriptionByName:key];
            if (typeDescriptor)
            {
                //为导出类型
                lua_pop(rawState, 1);
                
                //先检测类型是否已经导入
                LSCStringValue *typeNameValue = [LSCStringValue createValue:typeDescriptor.typeName];
                [typeNameValue pushWithContext:context];
                lua_rawget(rawState, 1);
                
                if (lua_isnil(rawState, -1))
                {
                    lua_pop(rawState, 1);
                    [apiAdapter pushType:typeDescriptor context:context];
                }
                
            }
        }
        
        [mainState unlock];
        
    }];
    
    return 1;
}

/**
 *  创建对象时处理
 *
 *  @param rawState 状态机
 *
 *  @return 参数数量
 */
static int objectCreateHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        [mainState lock];
        
        
        
        LSCTypeDescription *typeDesc = [LSCTypeValue createValueWithContext:context stackIndex:1].rawValue;
        if (!typeDesc)
        {
            [context raiseExceptionWithMessage:@"please use the colon syntax to call the method"];
        }
        else
        {
            
            
            NSArray<id<LSCValueType>> *arguments = [apiAdapter getArgumentsWithIndex:1 length:-1 context:context];

            //调用实例的init方法
            int errFuncIndex = [apiAdapter setExceptionHandlerWithContext:context];
            //先为实例对象在lua中创建内存
            LSCUserdataRef ref = lua_newuserdata(rawState, sizeof(LSCUserdataRef));
            
        
            NSString *instanceId = [apiAdapter getLuaObjectIdWithStackIndex:-1 context:context];
            LSCInstance *instance = [typeDesc constructInstanceWithInstanceId:instanceId
                                                                    arguments:arguments
                                                                      context:context];
            
            if (instance)
            {
                ref -> value = (__bridge_retained void *)instance;
                
                //创建一个临时table作为元表，用于在lua上动态添加属性或方法
                lua_newtable(rawState);
                
                ///变更索引为function，实现动态路由
                lua_pushlightuserdata(rawState, (__bridge void *)instance);
                lua_pushcclosure(rawState, instanceIndexHandler, 1);
                lua_setfield(rawState, -2, "__index");
                
                lua_pushlightuserdata(rawState, (__bridge void *)instance);
                lua_pushcclosure(rawState, instanceNewIndexHandler, 1);
                lua_setfield(rawState, -2, "__newindex");
                
                lua_pushlightuserdata(rawState, (__bridge void *)instance);
                lua_pushcclosure(rawState, objectDestroyHandler, 1);
                lua_setfield(rawState, -2, "__gc");
                
                lua_pushlightuserdata(rawState, (__bridge void *)instance);
                lua_pushcclosure(rawState, objectToStringHandler, 1);
                lua_setfield(rawState, -2, "__tostring");
                
                lua_pushvalue(rawState, -1);
                lua_setmetatable(rawState, -3);
                
                luaL_getmetatable(rawState, typeDesc.prototypeName.UTF8String);
                if (lua_istable(rawState, -1))
                {
                    lua_setmetatable(rawState, -2);
                }
                else
                {
                    //pop prototype metatable
                    lua_pop(rawState, 1);
                }
                
                //pop metatable
                lua_pop(rawState, 1);
                
                lua_getfield(rawState, -1, "init");
                if (lua_isfunction(rawState, -1))
                {
                    lua_pushvalue(rawState, -2);
                    
                    //将create传入的参数传递给init方法
                    //-4 代表有4个非参数值在栈中，由栈顶开始计算，分别是：实例对象，init方法，实例对象，异常捕获方法
                    int paramCount = lua_gettop(rawState) - 4;
                    //从索引2开始传入参数，ver2.2后使用冒号调用create方法，忽略第一个参数self
                    for (int i = 2; i <= paramCount; i++)
                    {
                        lua_pushvalue(rawState, i);
                    }
                    
                    lua_pcall(rawState, paramCount, 0, errFuncIndex);
                }
                else
                {
                    //pop init function
                    lua_pop(rawState, 1);
                }
                
                //移除异常捕获方法
                [apiAdapter removeExceptionHandlerWithIndex:errFuncIndex context:context];
            }
            else
            {
                lua_pop(rawState, 1);
                
                NSString *msg = [NSString stringWithFormat:@"Failed to construct an instance of `%@`", typeDesc.typeName];
                [context raiseExceptionWithMessage:msg];
                
                lua_pushnil(rawState);
            }
            
            
            
        }
        
        [mainState unlock];
        
    }];
    
    return 1;
}

/**
 类方法路由处理器
 
 @param rawState 状态
 @return 返回参数数量
 */
static int classMethodRouteHandler(lua_State *rawState)
{
    __block int retCount = 0;
    
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        id<LSCValueType> selfObjValue = [LSCValue createValueWithContext:context stackIndex:1];
        if (![selfObjValue isKindOfClass:[LSCTypeValue class]])
        {
            [context raiseExceptionWithMessage:@"please use the colon syntax to call the method"];
        }
        else
        {
            LSCTypeDescription *typeDesc = selfObjValue.rawValue;
            NSArray<id<LSCValueType>> *arguments = [apiAdapter getArgumentsWithIndex:2
                                                                              length:-1
                                                                             context:context];
            //获取方法名称
            id<LSCValueType> methodNameValue = [apiAdapter getUpvalueWithIndex:2 context:context];
            if ([methodNameValue isKindOfClass:[LSCStringValue class]])
            {
                NSString *methodName = methodNameValue.rawValue;
                id<LSCValueType> resultValue = [typeDesc callMethodWithName:methodName
                                                                  arguments:arguments
                                                                    context:context];
                
                if (resultValue)
                {
                    if ([resultValue isKindOfClass:[LSCTupleValue class]])
                    {
                        retCount = (int)((LSCTupleValue *)resultValue).count;
                    }
                    else
                    {
                        retCount = 1;
                    }
                    
                    [resultValue pushWithContext:context];
                }
                
            }
        }
        
    }];
    
    return retCount;
}

/**
 类型转换为字符串处理
 
 @param rawState 状态
 @return 参数数量
 */
static int classToStringHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        LSCTypeDescription *typeDesc = [apiAdapter getUpvalueWithIndex:1 context:context].rawValue;
        if (typeDesc)
        {
            lua_pushstring(rawState, typeDesc.description.UTF8String);
        }
        else
        {
            [context raiseExceptionWithMessage:@"can not describe unknown type."];
            lua_pushstring(rawState, "Unknown.");
        }
        
        [mainState unlock];
        
    }];
    
    return 1;
}


/**
 类型新增索引事件

 @param rawState 状态
 @return 返回值数量
 */
static int classNewIndexHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        //获取元表并进行值设置
        lua_getmetatable(rawState, 1);
        lua_pushvalue(rawState, 2);
        lua_pushvalue(rawState, 3);
        lua_rawset(rawState, -3);
        lua_pop(rawState, 1);
        
        [mainState unlock];
        
    }];
    
    return 0;
}


/**
 类型销毁事件

 @param rawState 状态
 @return 返回值数量
 */
static int classDestroyHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        if (lua_gettop(rawState) >= 1 && lua_isuserdata(rawState, 1))
        {
            //释放类型对象
            LSCUserdataRef ref = lua_touserdata(rawState, 1);
            CFBridgingRelease(ref -> value);
        }
        
        [mainState unlock];
        
    }];
    
    return 0;
}

/**
 *  子类化
 *
 *  @param rawState 状态机
 *
 *  @return 参数数量
 */
static int subClassHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        if (lua_gettop(rawState) < 2)
        {
            [context raiseExceptionWithMessage:@"missing parameter."];
        }
        else
        {
            id<LSCValueType> selfObjValue = [LSCValue createValueWithContext:context stackIndex:1];
            id<LSCValueType> typeNameValue = [LSCValue createValueWithContext:context stackIndex:2];
            
            if (![selfObjValue isKindOfClass:[LSCTypeValue class]])
            {
                [context raiseExceptionWithMessage:@"please use the colon syntax to call the method."];
            }
            else if (![typeNameValue isKindOfClass:[LSCStringValue class]])
            {
                [context raiseExceptionWithMessage:@"parameter type mismatch."];
            }
            else
            {
                LSCTypeDescription *typeDesc = selfObjValue.rawValue;
                NSString *typeName = typeNameValue.rawValue;
                
                LSCTypeDescription *subtypeDesc = [typeDesc subtypeWithName:typeName];
                id<LSCValueType> subtypeValue = [LSCValue createValue:subtypeDesc];
                
                [context setGlobal:subtypeValue forName:typeName];
            }
        }
        
        [mainState unlock];
        
    }];
    
    return 0;
}

/**
 判断是否是该类型的子类
 
 @param rawState 状态机
 @return 参数数量
 */
static int subclassOfHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        BOOL flag = NO;
        
        if (lua_gettop(rawState) < 2)
        {
            [context raiseExceptionWithMessage:@"missing parameter."];
        }
        else
        {
            id<LSCValueType> arg1 = [LSCValue createValueWithContext:context stackIndex:1];
            id<LSCValueType> arg2 = [LSCValue createValueWithContext:context stackIndex:2];
            
            if ([arg1 isKindOfClass:[LSCTypeValue class]] && [arg2 isKindOfClass:[LSCTypeValue class]])
            {
                LSCTypeDescription *typeDesc = arg1.rawValue;
                LSCTypeDescription *checkTypeDesc = arg2.rawValue;
                flag = [typeDesc subtypeOfType:checkTypeDesc];
            }
            else
            {
                [context raiseExceptionWithMessage:@"parameter type mismatch."];
            }
            
        }
        
        id<LSCValueType> flagValue = [LSCValue createValue:@(flag)];
        [flagValue pushWithContext:context];
        
        [mainState unlock];
        
    }];
    
    return 1;
}

/**
 类型别名
 
 @param rawState 状态
 @return 返回参数数量
 */
static int aliasHandler(lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        if (lua_gettop(rawState) < 2)
        {
            [context raiseExceptionWithMessage:@"missing parameter."];
        }
        else
        {
            id<LSCValueType> selfObjValue = [LSCValue createValueWithContext:context stackIndex:1];
            id<LSCValueType> nameValue = [LSCValue createValueWithContext:context stackIndex:2];
            if (![selfObjValue isKindOfClass:[LSCTypeValue class]])
            {
                [context raiseExceptionWithMessage:@"please use the colon syntax to call the method."];
            }
            else if (![nameValue isKindOfClass:[LSCStringValue class]])
            {
                [context raiseExceptionWithMessage:@"parameter type mismatch."];
            }
            else
            {
                NSString *alias = nameValue.rawValue;
                
                //由于改写了全局变量的__index方法，因此使用rawget判断是否存在该名称的变量比直接使用lua_getglobal的效率要高
                lua_getglobal(rawState, "_G");
                lua_pushstring(rawState, alias.UTF8String);
                lua_rawget(rawState, -2);
                
                if (lua_isnil(rawState, -1))
                {
                    //设置别名
                    lua_pop(rawState, 1);
                    [context setGlobal:selfObjValue forName:alias];
                }
                else
                {
                    lua_pop(rawState, 1);
                    
                    NSString *msg = [NSString stringWithFormat:@"`%@` already exists", alias];
                    [context raiseExceptionWithMessage:msg];
                }
                
                //pop _G
                lua_pop(rawState, 1);
                
            }
            
        }
        
        [mainState unlock];
        
    }];
    
    return 0;
}

/**
 设置原型的新属性处理
 
 @param rawState 状态
 @return 参数数量
 */
static int prototypeNewIndexHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        [mainState lock];
        
        //t,k,v
        BOOL isPropertyReg = NO;
        if (lua_istable(rawState, 3))
        {
            //检测是否为属性设置
            LSCFunctionValue *getter = nil;
            LSCFunctionValue *setter = nil;
            
            lua_getfield(rawState, 3, "get");
            if (lua_isfunction(rawState, -1))
            {
                getter = [LSCFunctionValue createValueWithContext:context stackIndex:-1];
            }
            lua_pop(rawState, 1);
            
            lua_getfield(rawState, 3, "set");
            if (lua_isfunction(rawState, -1))
            {
                setter = [LSCFunctionValue createValueWithContext:context stackIndex:-1];
            }
            lua_pop(rawState, 1);
            
            if (getter || setter)
            {
                isPropertyReg = YES;
                
                //注册属性
                LSCTypeDescription *typeDesc = [apiAdapter getUpvalueWithIndex:1 context:context].rawValue;
                if ([typeDesc isKindOfClass:[LSCTypeDescription class]])
                {
                    NSString *key = [LSCStringValue createValueWithContext:context stackIndex:2].rawValue;
                    [typeDesc registerPropertyWithName:key
                                            getterFunc:getter
                                            setterFunc:setter
                                               context:context];
                }
            }
        }
        
        if (!isPropertyReg)
        {
            //获取元表并进行值设置
            lua_getmetatable(rawState, 1);
            lua_pushvalue(rawState, 2);
            lua_pushvalue(rawState, 3);
            lua_rawset(rawState, -3);
            
            lua_pop(rawState, 1);
        }
        
        [mainState unlock];
        
    }];
    
    return 0;
}

/**
 转换Prototype为字符串处理
 
 @param rawState 状态
 @return 参数数量
 */
static int prototypeToStringHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        [mainState lock];
        
        LSCTypeDescription *typeDesc = [apiAdapter getUpvalueWithIndex:1 context:context].rawValue;
        if (typeDesc)
        {
            NSString *prototypeDesc = [NSString stringWithFormat:@"[%@ prototype]", typeDesc.typeName];
            lua_pushstring(rawState, prototypeDesc.UTF8String);
        }
        else
        {
            [context raiseExceptionWithMessage:@"can not describe unknown prototype."];
            lua_pushstring(rawState, "Unknown.");
        }
        
        [mainState unlock];
        
    }];
    
    return 1;
}

/**
 实例对象索引方法处理器
 
 @param rawState 状态
 @return 返回参数数量
 */
static int instanceIndexHandler(lua_State *rawState)
{
    __block int retValueCount = 1;
    
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        [mainState lock];
        
        LSCInstance *inst = [apiAdapter getUpvalueWithIndex:1 context:context].rawValue;
        if ([inst isKindOfClass:[LSCInstance class]])
        {
            NSString *key = [LSCValue createValueWithContext:context stackIndex:2].rawValue;
            BOOL hasExists = NO;
            
            //检测元表是否包含指定值
            lua_getmetatable(rawState, 1);
            int mtIndex = lua_absindex(rawState, lua_gettop(rawState));
            lua_getfield(rawState, -1, key.UTF8String);

            if (lua_isnil(rawState, -1))
            {
                lua_pop(rawState, 1);
            }
            else
            {
                hasExists = YES;
            }
            
            lua_remove(rawState, mtIndex);
            
            if (!hasExists)
            {
                id<LSCValueType> value = [inst getPropertyForKey:key context:context];
                if (value)
                {
                    if ([value isKindOfClass:[LSCTupleValue class]])
                    {
                        retValueCount = (int)((LSCTupleValue *)value).count;
                    }
                    
                    [value pushWithContext:context];
                }
                else
                {
                    lua_pushnil(rawState);
                }
            }
            
        }
        
        [mainState unlock];
        
    }];
    
    return retValueCount;
}

/**
 实例对象更新索引处理
 
 @param rawState 状态机
 @return 参数数量
 */
static int instanceNewIndexHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        [mainState lock];
        
        LSCInstance *inst = [LSCInstanceValue createValueWithContext:context stackIndex:1].rawValue;
        if (inst)
        {
            NSString *key = [LSCValue createValueWithContext:context stackIndex:2].rawValue;
            if ([inst.typeDescription existsPropertyeWithName:key])
            {
                id<LSCValueType> value = [LSCValue createValueWithContext:context stackIndex:3];
                [inst setProperty:value forKey:key context:context];
            }
            else
            {
                //先找到实例对象的元表，向元表添加属性
                lua_getmetatable(rawState, 1);
                if (lua_istable(rawState, -1))
                {
                    lua_pushvalue(rawState, 2);
                    lua_pushvalue(rawState, 3);
                    lua_rawset(rawState, -3);
                }
                lua_pop(rawState, 1);
            }
            
        }
        
        [mainState unlock];
        
    }];
    
    return 0;
}

/**
 判断是否是该类型的实例对象
 
 @param rawState 状态机
 @return 参数数量
 */
static int instanceOfHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        [mainState lock];
        
        BOOL flag = NO;
        if (lua_gettop(rawState) < 2)
        {
            [context raiseExceptionWithMessage:@"missing parameter."];
        }
        else
        {
            LSCInstance *inst = [LSCInstanceValue createValueWithContext:context stackIndex:1].rawValue;
            LSCTypeDescription *typeDesc = [LSCTypeValue createValueWithContext:context stackIndex:2].rawValue;
            if (inst && typeDesc)
            {
                flag = [inst.typeDescription subtypeOfType:typeDesc];
            }
            else
            {
                [context raiseExceptionWithMessage:@"parameter type mismatch."];
            }
        }
        
        id<LSCValueType> flagValue = [LSCValue createValue:@(flag)];
        [flagValue pushWithContext:context];
        
        [mainState unlock];
        
    }];
    
    return 1;
}


/**
 实例方法处理器

 @param rawState 状态
 @return 返回值数量
 */
static int instanceMethodRouteHandler(lua_State *rawState)
{
    return 0;
}

/**
 *  对象销毁处理
 *
 *  @param rawState 状态机
 *
 *  @return 参数数量
 */
static int objectDestroyHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        if (lua_gettop(rawState) >= 1 && lua_isuserdata(rawState, 1))
        {
            //调用destroy方法
            lua_getfield(rawState, 1, "destroy");
            LSCFunctionValue *destroyFunc = [LSCFunctionValue createValueWithContext:context stackIndex:-1];
            if (destroyFunc)
            {
                id<LSCValueType> instValue = [LSCValue createValueWithContext:context stackIndex:1];
                [destroyFunc invokeWithArguments:@[instValue] context:context];
            }
            else
            {
                lua_pop(rawState, 1);
            }
            
            //释放类型对象
            LSCUserdataRef ref = lua_touserdata(rawState, 1);
            CFBridgingRelease(ref -> value);
        }
        
        [mainState unlock];
        
    }];
    
    return 0;
}

/**
 *  对象转换为字符串处理
 *
 *  @param rawState 状态机
 *
 *  @return 参数数量
 */
static int objectToStringHandler (lua_State *rawState)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        [mainState lock];
        
        LSCInstance *obj = [apiAdapter getUpvalueWithIndex:1 context:context].rawValue;
        if (obj)
        {
            lua_pushstring(rawState, obj.description.UTF8String);
        }
        else
        {
            [context raiseExceptionWithMessage:@"can not describe unknown object."];
            lua_pushstring(rawState, "Unknown.");
        }
        
        [mainState unlock];
        
    }];
    
    return 1;
}

@end
