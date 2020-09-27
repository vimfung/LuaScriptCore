//
//  LSCApiAdapter.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCApiAdapter.h"
#import "LSCContext+Private.h"
#import "LSCCoroutine.h"
#import "lauxlib.h"
#import "luaconf.h"
#import "lualib.h"
#import "lapi.h"
#import "ldo.h"
#import "LSCTupleValue.h"
#import "LSCValue.h"
#import "LSCNumberValue.h"
#import "LSCStringValue.h"
#import "LSCTypeDescription.h"
#import "LSCFunctionValue+Private.h"
#import "LSCMainState.h"
#import "LSCStateWatcher.h"

static Class _apiAdapterClass = nil;
static LSCApiAdapter *_defaultApiAdapter = nil;

/**
 捕获Lua异常处理器名称
 */
static NSString *const LSCCatchLuaExceptionHandlerName = @"__catchExcepitonHandler";

/**
 记录弱引用的Lua变量表名称
 */
static char *const LSCWeakVarsTableName = "_weakVars_";

/**
 记录保留的Lua变量表名称
 */
static char *const LSCRetainVarsTableName = "_retainVars_";

@interface LSCApiAdapter ()
{
@private
    pthread_mutex_t _mutexHandle;      //锁，用于控制api的操作同步
}

@end

@implementation LSCApiAdapter

+ (void)registerApiAdapterClass:(Class)adapterClass
{
    @synchronized (self)
    {
        if ([adapterClass isKindOfClass:[LSCApiAdapter class]]
            && _apiAdapterClass != adapterClass)
        {
            _apiAdapterClass = adapterClass;
            _defaultApiAdapter = [[_apiAdapterClass alloc] init];   //重置对象
        }
    }
}

+ (LSCApiAdapter *)defaultApiAdapter
{
    @synchronized (self) {
        
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            
            if (!_defaultApiAdapter)
            {
                if (!_apiAdapterClass)
                {
                    _apiAdapterClass = [LSCApiAdapter class];
                }
                _defaultApiAdapter = [[_apiAdapterClass alloc] init];
            }
            
        });
        
        return _defaultApiAdapter;
    }
}

+ (void)handleClosureWithRawState:(lua_State *)rawState
                            block:(LSCClosureHandler)block
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    LSCState *state = [LSCState stateWithRawState:rawState];
    
    LSCMainState *mainState = [apiAdapter getMainStateWithState:state];
    LSCContext *context = mainState.context;
    
    [mainState mount:state];

    if (block)
    {
        block (apiAdapter, context, mainState, state);
    }
    
    [mainState unmount];
}

- (lua_State *)createState
{
    //创建状态的操作不存在线程冲突问题，因此不需要加锁。
    
    lua_State *state = luaL_newstate();
    lua_gc(state, LSCGCTypeStop, 0);
    
    //加载标准库
    luaL_openlibs(state);
    lua_gc(state, LSCGCTypeRestart, 0);
    
    return state;
}

- (NSString *)createThread:(lua_State **)threadState
                   context:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    *threadState = lua_newthread(rawState);
    
    //引用线程状态
    NSString *objId = [self getLuaObjectIdWithStackIndex:-1 context:context];
    [self setLuaObjectWithId:objId option:LSCSetLuaObjectOptionRetain context:context];
    
    //移除栈中线程状态
    lua_pop(rawState, 1);
    
    [mainState unlock];
    
    return objId;
}

- (void)closeThreadWithId:(NSString *)threadId context:(LSCContext *)context
{
    if (threadId)
    {
        [self setLuaObjectWithId:threadId
                          option:LSCSetLuaObjectOptionRelease
                         context:context];
    }
}

- (void)resumeWithCoroutine:(LSCCoroutine *)coroutine
                  arguments:(nonnull NSArray<id<LSCValueType>> *)arguments
{
    dispatch_async(coroutine.queue, ^{
        
        id<LSCValueType> returnValue = nil;
        
        LSCContext *context = coroutine.context;
        LSCMainState *mainState = context.mainState;
        //挂载协程
        [mainState mount:coroutine];
        [mainState lock];
        
        lua_State *rawState = coroutine.rawState;
        int status = lua_status(rawState);
        
        if (status != LUA_OK && status != LUA_YIELD)
        {
            return;
        }
        
        int curTop = lua_gettop(rawState);
        int returnCount = 0;
        
        if (status == LUA_OK)
        {
            [coroutine.handler pushWithContext:context];
        }
        
        [arguments enumerateObjectsUsingBlock:^(id<LSCValueType>  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
            
            [obj pushWithContext:context];
            
        }];
        
        [mainState unlock];
        
        //执行协程时不进行加锁，否则其他线程需要等待协程完成后运行
        status = lua_resume(rawState, mainState.rawState, (int)arguments.count);
        
        [mainState lock];
        
        if (status == LUA_OK || status == LUA_YIELD)
        {
            returnCount = lua_gettop(rawState) - curTop;
            if (returnCount > 1)
            {
                //返回值为元组
                returnValue = [[LSCTupleValue alloc] initWithContext:context
                                                          stackIndex:curTop
                                                               count:returnCount];
            }
            else if (returnCount == 1)
            {
                //单个返回值
                returnValue = [LSCValue createValueWithContext:context stackIndex:-1];
            }
        }
        else
        {
            //调用失败
            returnCount = lua_gettop(rawState) - curTop;
        }
        
        lua_pop(rawState, returnCount);
        
        [context gc];
        
        [mainState unlock];
        
        //卸载协程
        [mainState unmount];
        
        if (coroutine.resultHandler)
        {
            coroutine.resultHandler(coroutine, returnValue);
        }
        
    });
}

- (LSCMainState *)getMainStateWithState:(LSCState *)state
{
    global_State *globalState = G(state.rawState);
    LSCState *mainState = [LSCState stateWithRawState:globalState -> mainthread];
    return (LSCMainState *)mainState;
}

- (void)closeContext:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];

    lua_close(mainState.rawState);
    
    [mainState unlock];
}

- (void)addSearchPath:(NSString *)path
              context:(LSCContext *)context
{
    NSMutableString *fullPath = [path mutableCopy];
    
    NSRegularExpression *regExp = [[NSRegularExpression alloc] initWithPattern:@"/([^/]+)[.]([^/]+)$" options:NSRegularExpressionCaseInsensitive error:nil];
    NSTextCheckingResult *result = [regExp firstMatchInString:path options:NSMatchingReportProgress range:NSMakeRange(0, fullPath.length)];
    
    if (!result)
    {
        if (![path hasSuffix:@"/"])
        {
            [fullPath appendString:@"/"];
        }
        [fullPath appendString:@"?.lua"];
    }
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.rawState;
    lua_getglobal(rawState, "package");
    lua_getfield(rawState, -1, "path");
    
    //取出当前路径，并附加新路径
    NSMutableString *curPath =
    [NSMutableString stringWithUTF8String:lua_tostring(rawState, -1)];
    [curPath appendFormat:@";%@", path];
    
    lua_pop(rawState, 1);
    lua_pushstring(rawState, curPath.UTF8String);
    lua_setfield(rawState, -2, "path");
    lua_pop(rawState, 1);
    
    [mainState unlock];
}

- (void)registerErrorFunction:(LSCFunctionValue *)errFunc
                      context:(LSCContext *)context
{
    [context setGlobal:errFunc forName:LSCCatchLuaExceptionHandlerName];
}

- (void)raiseErrorWithMessage:(NSString *)message
                      context:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    luaD_rawrunprotected(rawState, raiseLuaExceptionHandler, (void *)message.UTF8String);
    
    [mainState unlock];
}

- (void)interruptWithMessage:(NSString *)message
                     context:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    lua_State *rawState = mainState.currentState.rawState;
    luaL_error(rawState, message.UTF8String);
}

- (LSCBasicType)getTypeWithStackIndex:(int)stackIndex
                              context:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    int type = lua_type(rawState, stackIndex);
    
    [mainState unlock];
    
    return type;
}

- (NSData *)getDataWithStackIndex:(int)stackIndex
                          context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    size_t len = 0;
    const char *bytes = lua_tolstring(rawState, stackIndex, &len);
    NSData *data = [NSData dataWithBytes:bytes length:len];
    
    [mainState unlock];
    
    return data;
}

- (NSNumber *)getNumberWithStackIndex:(int)stackIndex
                              context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_Number value = lua_tonumber(rawState, stackIndex);
    
    [mainState unlock];
    
    return @(value);
}

- (NSNumber *)getBooleanWithStackIndex:(int)stackIndex
                               context:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    int value = lua_toboolean(rawState, stackIndex);
    
    [mainState unlock];
    
    BOOL boolValue = value == 1 ? YES : NO;
    return @(boolValue);
}

- (id)getTableWithStackIndex:(int)stackIndex
                     context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;

    //为Table数据
    NSMutableDictionary *dictValue = [NSMutableDictionary dictionary];
    NSMutableArray *arrayValue = [NSMutableArray array];
    
    stackIndex = lua_absindex(rawState, stackIndex);
    
    lua_pushnil(rawState);
    while (lua_next(rawState, stackIndex))
    {
        id<LSCValueType> value = [LSCValue createValueWithContext:context stackIndex:-1];
        id<LSCValueType> key = [LSCValue createValueWithContext:context stackIndex:-2];
        
        if (arrayValue)
        {
            if (![key isKindOfClass:[LSCNumberValue class]])
            {
                //非数组对象，释放数组
                arrayValue = nil;
            }
            else if ([key isKindOfClass:[LSCNumberValue class]])
            {
                NSInteger index = [key.rawValue integerValue];
                if (index <= 0)
                {
                    //非数组对象，释放数组
                    arrayValue = nil;
                }
                else if (index - 1 != arrayValue.count)
                {
                    //非数组对象，释放数组
                    arrayValue = nil;
                }
                else
                {
                    [arrayValue addObject:value.rawValue];
                }
            }
        }
        
        NSString *keyString = nil;
        if ([key isKindOfClass:[LSCStringValue class]])
        {
            keyString = key.rawValue;
        }
        else
        {
            keyString = [NSString stringWithFormat:@"%@", key.rawValue];
        }
        
        if (keyString)
        {
            id valueObj = value.rawValue;
            if ([value isKindOfClass:[LSCStringValue class]])
            {
                //尝试将字符串数据进行转换
                NSString *string = value.rawValue;
                if (string)
                {
                    valueObj = string;
                }
            }
            
            [dictValue setObject:valueObj forKey:keyString];
        }
        
        lua_pop(rawState, 1);
    }
    
    [mainState unlock];
    
    return arrayValue && arrayValue.count > 0 ? arrayValue : dictValue;
}

- (id)getObjectWithStackIndex:(int)stackIndex
                      context:(nonnull LSCContext *)context
{
    id obj = nil;
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    LSCBasicType type = lua_type(rawState, stackIndex);
    
    const void *value = lua_topointer(rawState, stackIndex);
    if (type == LSCBasicTypeLightUserdata)
    {
        obj = (__bridge id _Nonnull)(value);
    }
    else if (type == LSCBasicTypeUserdata)
    {
        LSCUserdataRef userdataRef = (LSCUserdataRef)value;
        obj = (__bridge id _Nonnull)userdataRef -> value;
    }
    
    [mainState unlock];
    
    return obj;
}

- (id<LSCValueType>)getUpvalueWithIndex:(int)index
                                context:(nonnull LSCContext *)context
{
    //该方法不涉及lua_State方法操作，不需要加锁
    index = lua_upvalueindex(index);
    return [LSCValue createValueWithContext:context stackIndex:index];
}

- (NSArray<id<LSCValueType>> *)getArgumentsWithIndex:(int)index
                                              length:(int)length
                                             context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    NSMutableArray *arguments = nil;
    
    lua_State *rawState = mainState.currentState.rawState;
    int top = lua_gettop(rawState);
    
    if (top >= index)
    {
        if (length == -1)
        {
            length = top + 1;
        }
        else
        {
            length = index + length;
        }
        
        arguments = [NSMutableArray array];
        for (int i = index; i <= top && i < length; i++)
        {
            id<LSCValueType> value = [LSCValue createValueWithContext:context stackIndex:i];
            [arguments addObject:value];
        }
    }
    
    [mainState unlock];
    
    return arguments;
}

- (void)pushNilWithContext:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_pushnil(rawState);
    
    [mainState unlock];
}

- (void)pushData:(NSData *)data context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_pushlstring(rawState, data.bytes, data.length);
    
    [mainState unlock];
}

- (void)pushNumber:(NSNumber *)number context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_pushnumber(rawState, number.doubleValue);
    
    [mainState unlock];
}

- (void)pushBoolean:(NSNumber *)boolean context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_pushboolean(rawState, boolean.boolValue);
    
    [mainState unlock];
}

- (void)pushArray:(NSArray *)array context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_newtable(rawState);
    
    [array enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
       
        id<LSCValueType> value = [LSCValue createValue:obj];
        if (value)
        {
            [value pushWithContext:context];
            lua_rawseti(rawState, -2, idx + 1);
        }
        else
        {
            NSLog(@"[WARNING] Object `%@` cannot be pushed", obj);
            *stop = YES;
        }
        
    }];
    
    [mainState unlock];
}

- (void)pushDictionary:(NSDictionary *)dictionary context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_newtable(rawState);
    
    [dictionary enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        
        id<LSCValueType> value = [LSCValue createValue:obj];
        if (value)
        {
            NSString *keyString = [NSString stringWithFormat:@"%@", key];
            [value pushWithContext:context];
            lua_setfield(rawState, -2, keyString.UTF8String);
        }
        else
        {
            NSLog(@"[WARNING] Object `%@` cannot be pushed", obj);
            *stop = YES;
        }
        
    }];
    
    [mainState unlock];
}

- (void)pushFunction:(LSCFunctionValue *)function context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_pushlightuserdata(rawState, (__bridge void *)(function));
    lua_pushcclosure(rawState, methodRouteHandler, 1);
    
    [mainState unlock];
}

- (void)pushUnknowObject:(id)object
                 context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_pushlightuserdata(rawState, (__bridge void *)object);
    
    [mainState unlock];
}

- (NSString *)getLuaObjectIdWithStackIndex:(int)stackIndex
                                   context:(LSCContext *)context
{
    NSString *objectId = nil;
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    stackIndex = lua_absindex(rawState, stackIndex);
    
    if (lua_type(rawState, stackIndex) != LSCBasicTypeNil)
    {
        objectId = [NSString stringWithFormat:@"%p", lua_topointer(rawState, stackIndex)];
        
        //将对象放入放入弱引用表
        lua_getglobal(rawState, "_G");
        if (lua_istable(rawState, -1))
        {
            //检测是否为弱引用，使用rawget获取提升效率
            lua_pushstring(rawState, LSCWeakVarsTableName);
            lua_rawget(rawState, -2);
            
            if (!lua_istable(rawState, -1))
            {
                //不存在弱引用表则进行创建
                lua_pop(rawState, 1);
                
                //创建引用表
                lua_newtable(rawState);
                
                //创建弱引用表元表
                lua_newtable(rawState);
                lua_pushstring(rawState, "kv");
                lua_setfield(rawState, -2, "__mode");
                lua_setmetatable(rawState, -2);
                
                //放入全局变量_G中
                lua_pushvalue(rawState, -1);
                lua_setfield(rawState, -3, LSCWeakVarsTableName);
            }
            
            //将对象放入弱引用表
            lua_pushvalue(rawState, stackIndex);
            lua_setfield(rawState, -2, objectId.UTF8String);
            
            //pop _weakVars_
            lua_pop(rawState, 1);
        }
        
        //pop _G
        lua_pop(rawState, 1);
    }
    
    [mainState unlock];
    
    return objectId;
}

- (void)setLuaObjectWithId:(NSString *)objectId
                    option:(LSCSetLuaObjectOption)option
                   context:(LSCContext *)context
{
    if (!objectId)
    {
        return;
    }
    
    if (option != LSCSetLuaObjectOptionRetain
        && option != LSCSetLuaObjectOptionRelease)
    {
        //weak操作在getObjectId时已经放入弱引用表中
        return;
    }
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    lua_getglobal(rawState, "_G");
    if (lua_istable(rawState, -1))
    {
        lua_pushstring(rawState, LSCRetainVarsTableName);
        lua_rawget(rawState, -2);
        if (!lua_istable(rawState, -1))
        {
            //创建_retainVars_
            lua_pop(rawState, 1);
            
            lua_newtable(rawState);
            
            lua_pushvalue(rawState, -1);
            lua_setfield(rawState, -3, LSCRetainVarsTableName);
        }
        
        switch (option) {
            case LSCSetLuaObjectOptionRetain:
            {
                //引用对象
                lua_getfield(rawState, -1, objectId.UTF8String);
                if (lua_isnil(rawState, -1))
                {
                    lua_pop(rawState, 1);
                    
                    lua_newtable(rawState);
                    
                    //初始化引用次数
                    lua_pushnumber(rawState, 0);
                    lua_setfield(rawState, -2, "rc");
                    
                    //放入对象
                    [self pushLuaObjectWithId:objectId context:context];
                    lua_setfield(rawState, -2, "o");
                    
                    //将对象放入表中
                    lua_pushvalue(rawState, -1);
                    lua_setfield(rawState, -3, objectId.UTF8String);
                }
                
                //引用次数+1
                lua_getfield(rawState, -1, "rc");
                lua_Integer retainCount = lua_tointeger(rawState, -1);
                lua_pop(rawState, 1);
                
                lua_pushnumber(rawState, retainCount + 1);
                lua_setfield(rawState, -2, "rc");
                
                //pop object
                lua_pop(rawState, 1);
                
                break;
            }
            case LSCSetLuaObjectOptionRelease:
            {
                lua_getfield(rawState, -1, objectId.UTF8String);
                if (!lua_isnil(rawState, -1))
                {
                    //引用次数-1
                    lua_getfield(rawState, -1, "rc");
                    lua_Integer retainCount = lua_tointeger(rawState, -1);
                    lua_pop(rawState, 1);
                    
                    retainCount --;
                    if (retainCount > 0)
                    {
                        lua_pushnumber(rawState, retainCount);
                        lua_setfield(rawState, -2, "rc");
                    }
                    else
                    {
                        //retainCount<=0时移除对象引用
                        lua_pushnil(rawState);
                        lua_setfield(rawState, -3, objectId.UTF8String);
                    }
                }
                
                //pop object
                lua_pop(rawState, 1);
                break;
            }
            default:
                break;
        }
        
        //pop _retainVars_
        lua_pop(rawState, 1);
        
    }
    
    //pop _G
    lua_pop(rawState, 1);
    
    [mainState unlock];
}

- (void)pushLuaObjectWithId:(NSString *)objectId
                    context:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    if (!objectId)
    {
        lua_pushnil(rawState);
    }
    else
    {
        lua_getglobal(rawState, "_G");
        if (lua_istable(rawState, -1))
        {
            BOOL hasExists = NO;
            
            //先查找弱引用表
            lua_getfield(rawState, -1, LSCWeakVarsTableName);
            if (lua_istable(rawState, -1))
            {
                lua_getfield(rawState, -1, objectId.UTF8String);
                if (!lua_isnil(rawState, -1))
                {
                    hasExists = YES;
                    
                    //pop _weakVars_，保留值在栈上
                    lua_remove(rawState, -2);
                }
                else
                {
                    //pop object、 _weakVars_
                    lua_pop(rawState, 2);
                }
            }
            else
            {
                //pop _weakVars_
                lua_pop(rawState, 1);
            }
            
            if (!hasExists)
            {
                //查找引用表
                lua_getfield(rawState, -1, LSCRetainVarsTableName);
                if (lua_istable(rawState, -1))
                {
                    lua_getfield(rawState, -1, objectId.UTF8String);
                    if (lua_istable(rawState, -1))
                    {
                        hasExists = YES;
                        lua_getfield(rawState, -1, "o");
                        
                        lua_insert(rawState, -3);
                    }
                    
                    //pop object_table、 _retainVars_
                    lua_pop(rawState, 2);
                }
                else
                {
                    //pop _retainVars_
                    lua_pop(rawState, 1);
                }
            }
            
            if (!hasExists)
            {
                //两个表都没有则使用nil填充
                lua_pushnil(rawState);
            }
        }
        
        //pop _G
        lua_remove(rawState, -2);
    }
    
    [mainState unlock];
}

- (int)setExceptionHandlerWithContext:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    int errFunIndex = 0;
    lua_getglobal(rawState, LSCCatchLuaExceptionHandlerName.UTF8String);
    if (lua_isfunction(rawState, -1))
    {
        errFunIndex = lua_gettop(rawState);
    }
    else
    {
        lua_pop(rawState, 1);
    }
    
    [mainState unlock];
    
    return errFunIndex;
}

- (void)removeExceptionHandlerWithIndex:(int)index
                                context:(LSCContext *)context
{
    if (index != 0)
    {
        LSCMainState *mainState = context.mainState;
        [mainState lock];
        lua_State *rawState = mainState.currentState.rawState;
        lua_remove(rawState, index);
        [mainState unlock];
    }
}

- (id<LSCValueType>)evalScriptWithString:(NSString *)script
                                 context:(nonnull LSCContext *)context
{
    if (!script)
    {
        @throw SCRIPT_CANNOT_BE_NIL_EXCEPTION;
    }
    
    id<LSCValueType> returnValue = nil;
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    int errFunIndex = [self setExceptionHandlerWithContext:context];
    
    //准备解析脚本
    int curTop = lua_gettop(rawState);
    int returnCount = 0;

    luaL_loadstring(rawState, script.UTF8String);
    if (lua_pcall(rawState, 0, LUA_MULTRET, errFunIndex) == LUA_OK)
    {
        //调用成功
        returnCount = lua_gettop(rawState) - curTop;
        if (returnCount > 1)
        {
            //返回值为元组
            returnValue = [[LSCTupleValue alloc] initWithContext:context
                                                      stackIndex:curTop
                                                           count:returnCount];
        }
        else if (returnCount == 1)
        {
            //单个返回值
            returnValue = [LSCValue createValueWithContext:context stackIndex:-1];
        }
    }
    else
    {
        //调用失败
        returnCount = lua_gettop(rawState) - curTop;
    }
    
    //根据返回值数量出栈
    lua_pop(rawState, returnCount);
    
    //移除捕获异常方法
    [self removeExceptionHandlerWithIndex:errFunIndex context:context];
    
    [context gc];

    [mainState unlock];
    
    return returnValue;
}

- (id<LSCValueType>)evalScriptWithPath:(NSString *)path
                               context:(nonnull LSCContext *)context
{
    if (!path)
    {
        @throw SCRIPT_PATH_INVALID_EXCEPTION;
    }
    
    if (![path hasPrefix:@"/"])
    {
        //应用包内路径
        path = [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] resourcePath], path];
    }
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:path])
    {
        @throw SCRIPT_PATH_INVALID_EXCEPTION;
    }
    
    id<LSCValueType> returnValue = nil;
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    //异常处理方法
    int errFunIndex = [self setExceptionHandlerWithContext:context];
    
    //准备解析脚本
    int curTop = lua_gettop(rawState);
    int returnCount = 0;
    
    luaL_loadfile(rawState, path.UTF8String);
    if (lua_pcall(rawState, 0, LUA_MULTRET, errFunIndex) == LUA_OK)
    {
        //调用成功
        returnCount = lua_gettop(rawState) - curTop;
        if (returnCount > 1)
        {
            //返回值为元组
            returnValue = [[LSCTupleValue alloc] initWithContext:context
                                                      stackIndex:curTop
                                                           count:returnCount];
        }
        else if (returnCount == 1)
        {
            //单个返回值
            returnValue = [LSCValue createValueWithContext:context stackIndex:-1];
        }
    }
    else
    {
        //调用失败
        returnCount = lua_gettop(rawState) - curTop;
    }

    //弹出返回值
    lua_pop(rawState, returnCount);
    
    //移除异常捕获方法
    [self removeExceptionHandlerWithIndex:errFunIndex context:context];
    
    //使用LSCState的gc方法可以延迟进行回收提高执行效率。
    [context gc];
    
    [mainState unlock];
    
    return returnValue;
}

- (void)setGlobalWithValue:(id<LSCValueType>)value
                      name:(NSString *)name
                   context:(nonnull LSCContext *)context
{
    if ([value isKindOfClass:[LSCTupleValue class]])
    {
        @throw TUPLE_CANNOT_BE_SET_EXCEPTION;
    }
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    if (!value)
    {
        value = [LSCValue createValue:nil];
    }
    
    [value pushWithContext:context];
    lua_setglobal(rawState, name.UTF8String);
    
    [mainState unlock];
}

- (id<LSCValueType>)getGlobalWithName:(NSString *)name
                              context:(nonnull LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    lua_getglobal(rawState, name.UTF8String);
    id<LSCValueType> value = [LSCValue createValueWithContext:context stackIndex:-1];
    lua_pop(rawState, 1);
    
    [mainState unlock];
    
    return value;
}

- (id<LSCValueType>)callFunctionWithId:(NSString *)functionId
                             arguments:(NSArray<id<LSCValueType>> *)arguments
                               context:(LSCContext *)context
{
    id<LSCValueType> returnValue = nil;
    
    if (functionId)
    {
        LSCMainState *mainState = context.mainState;
        [mainState lock];
        
        lua_State *rawState = mainState.currentState.rawState;
        
        //异常处理方法
        int errFunIndex = [self setExceptionHandlerWithContext:context];
        int curTop = lua_gettop(rawState);
        
        [self pushLuaObjectWithId:functionId context:context];
        if (lua_isfunction(rawState, -1))
        {
            int returnCount = 0;
            
            //如果为function则进行调用
            
            //入栈参数
            [arguments enumerateObjectsUsingBlock:^(id<LSCValueType> arg, NSUInteger idx, BOOL *_Nonnull stop) {
                
                [arg pushWithContext:context];
                
            }];
            
            if (lua_pcall(rawState, (int)arguments.count, LUA_MULTRET, errFunIndex) == LUA_OK)
            {
                returnCount = lua_gettop(rawState) - curTop;
                if (returnCount > 1)
                {
                    //返回值为元组
                    returnValue = [[LSCTupleValue alloc] initWithContext:context
                                                              stackIndex:curTop
                                                                   count:returnCount];
                }
                else if (returnCount == 1)
                {
                    //单个返回值
                    returnValue = [LSCValue createValueWithContext:context stackIndex:-1];
                }
            }
            else
            {
                //调用失败
                returnCount = lua_gettop(rawState) - curTop;
            }
            
            lua_pop(rawState, returnCount);
        }
        else
        {
            lua_pop(rawState, 1);
        }
        
        [self removeExceptionHandlerWithIndex:errFunIndex context:context];
        
        //使用LSCState的gc方法可以延迟进行回收提高执行效率。
        [context gc];
        
        [mainState unlock];
    }
    
    return returnValue;
}

- (void)gcWithContext:(LSCContext *)context
{
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.rawState;
    lua_gc(rawState, LSCGCTypeCollect, 0);
    
    [mainState unlock];
}

- (void)startWatchEvent:(LSCWatchEvents)event
                  state:(LSCState *)state
                  count:(NSInteger)count
{
    LSCMainState *mainState = [self getMainStateWithState:state];
    [mainState lock];
    
    lua_State *rawState = state.rawState;
    lua_sethook(rawState, hookFunc, event, (int)count);
    
    [mainState unlock];
}

- (void)stopWatchEventWithstate:(LSCState *)state
{
    LSCMainState *mainState = [self getMainStateWithState:state];
    [mainState lock];
    
    lua_State *rawState = state.rawState;
    lua_sethook(rawState, hookFunc, 0, 0);
    
    [mainState unlock];
}

- (void)setTableValue:(id<LSCValueType>)value
           forKeyPath:(NSString *)keyPath
               withId:(NSString *)tableId
              context:(LSCContext *)context
{
    NSArray<NSString *> *keys = [keyPath componentsSeparatedByString:@"."];
    
    LSCMainState *mainState = context.mainState;
    [mainState lock];
    
    lua_State *rawState = mainState.currentState.rawState;
    
    [self pushLuaObjectWithId:tableId context:context];
    if (lua_istable(rawState, -1))
    {
        //先寻找对应的table对象
        BOOL hasExists = YES;
        if (keys.count > 1)
        {
            for (NSInteger i = 0; i < keys.count - 1; i++)
            {
                NSString *key = keys[i];
                lua_pushstring(rawState, key.UTF8String);
                lua_rawget(rawState, -2);
                
                if (lua_istable(rawState, -1))
                {
                    lua_remove(rawState, -2);
                }
                else
                {
                    hasExists = NO;
                    lua_pop(rawState, 1);
                    break;
                }
            }
        }
        
        //设置对象
        if (hasExists)
        {
            lua_pushstring(rawState, keys.lastObject.UTF8String);
            [value pushWithContext:context];
            lua_rawset(rawState, -3);
        }
    }
    lua_pop(rawState, 1);
    
    [mainState unlock];
    
}

#pragma mark - C Function

/**
 抛出Lua异常
 
 @param rawState 状态
 @param ud 异常信息
 */
static void raiseLuaExceptionHandler(lua_State *rawState, void *ud)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        const char *msg = (const char *)ud;
        luaL_error(rawState, msg);
        
    }];
}

/**
 C方法路由处理器
 
 @param rawState 状态
 @return 参数数量
 */
static int methodRouteHandler(lua_State *rawState)
{
    __block int count = 0;
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
        
        LSCFunctionValue *functionValue = [apiAdapter getUpvalueWithIndex:1 context:context].rawValue;
        if (functionValue.handler)
        {
            NSArray<id<LSCValueType>> *arguments = [apiAdapter getArgumentsWithIndex:1 length:-1 context:context];
            id<LSCValueType> retValue = functionValue.handler(arguments);
            
            //返回值
            if (retValue)
            {
                if ([retValue isKindOfClass:[LSCTupleValue class]])
                {
                    count = (int)((LSCTupleValue *)retValue).count;
                }
                else
                {
                    count = 1;
                }
                
                [retValue pushWithContext:context];
                
            }
        }
        
    }];
    
    return count;
}

static void hookFunc(lua_State *rawState, lua_Debug *ar)
{
    [LSCApiAdapter handleClosureWithRawState:rawState block:^(LSCApiAdapter *apiAdapter, LSCContext *context, LSCMainState *mainState, LSCState *curState) {
       
        LSCStateWatcher *watcher = curState.watcher;
        [watcher dispatchEvent:context mainState:mainState curState:curState];
        
    }];
}

@end
