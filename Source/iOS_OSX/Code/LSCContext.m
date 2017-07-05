//
//  LUAContext.m
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCContext.h"
#import "LSCContext_Private.h"
#import "LSCValue_Private.h"
#import "LSCModule_Private.h"
#import "LSCSession_Private.h"
#import "LSCSession_Private.h"
#import "LSCTuple.h"
#import <objc/runtime.h>

@implementation LSCContext

- (instancetype)init
{
    if (self = [super init])
    {
        self.methodBlocks = [NSMutableDictionary dictionary];
        
        lua_State *state = luaL_newstate();
        
        lua_gc(state, LUA_GCSTOP, 0);
        
        //加载标准库
        luaL_openlibs(state);
        
        lua_gc(state, LUA_GCRESTART, 0);
        
        //创建主会话
        self.mainSession = [[LSCSession alloc] initWithState:state context:self];
        
        //设置搜索路径
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        [self addSearchPath:resourcePath];
        
        //初始化数据交换器
        self.dataExchanger = [[LSCDataExchanger alloc] initWithContext:self];
        
        
    }
    
    return self;
}

- (void)dealloc
{
    lua_close(self.mainSession.state);
}

- (LSCSession *)currentSession
{
    if (_currentSession)
    {
        return _currentSession;
    }
    
    return _mainSession;
}

- (void)onException:(LSCExceptionHandler)handler
{
    self.exceptionHandler = handler;
}

- (void)addSearchPath:(NSString *)path
{
    [self setSearchPath:[NSString stringWithFormat:@"%@/?.lua", path]];
}

- (void)setGlobalWithValue:(LSCValue *)value forName:(NSString *)name
{
    [value pushWithContext:self];
    lua_setglobal(self.mainSession.state, name.UTF8String);
}

- (LSCValue *)getGlobalForName:(NSString *)name
{
    lua_getglobal(self.mainSession.state, name.UTF8String);
    return [LSCValue valueWithContext:self atIndex:-1];
}

- (void)retainValue:(LSCValue *)value
{
    [self.dataExchanger retainLuaObject:value];
}

- (void)releaseValue:(LSCValue *)value
{
    [self.dataExchanger releaseLuaObject:value];
}

- (LSCValue *)evalScriptFromString:(NSString *)string
{
    lua_State *state = self.mainSession.state;
    
    LSCValue *returnValue = nil;
    int curTop = lua_gettop(state);
    int returnCount = 0;
    
    luaL_loadstring(state, [string UTF8String]);
    if (lua_pcall(state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(state) - curTop;
        if (returnCount > 1)
        {
            LSCTuple *tuple = [[LSCTuple alloc] init];
            for (int i = 1; i <= returnCount; i++)
            {
                LSCValue *value = [LSCValue valueWithContext:self atIndex:curTop + i];
                [tuple addReturnValue:[value toObject]];
            }
            
            returnValue = [LSCValue tupleValue:tuple];
        }
        else if (returnCount == 1)
        {
            returnValue = [LSCValue valueWithContext:self atIndex:-1];
        }
    }
    else
    {
        //调用失败
        returnCount = 1;
        LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
        NSString *errMessage = [value toString];
        [self raiseExceptionWithMessage:errMessage];
    }
    
    //弹出返回值
    lua_pop(state, returnCount);
    
    if (!returnValue)
    {
        returnValue = [LSCValue nilValue];
    }
    
    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return returnValue;
}

- (LSCValue *)evalScriptFromFile:(NSString *)path
{
    if (!path)
    {
        NSString *errMessage = @"Lua file path is empty!";
        [self raiseExceptionWithMessage:errMessage];
        
        return nil;
    }
    
    if (![path hasPrefix:@"/"])
    {
        //应用包内路径
        path = [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] resourcePath], path];
    }
    
    lua_State *state = self.mainSession.state;
    
    LSCValue *retValue = nil;
    int curTop = lua_gettop(state);
    int returnCount = 0;
    
    luaL_loadfile(state, [path UTF8String]);
    if (lua_pcall(state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(state) - curTop;
        if (returnCount > 1)
        {
            LSCTuple *tuple = [[LSCTuple alloc] init];
            for (int i = 1; i <= returnCount; i++)
            {
                LSCValue *value = [LSCValue valueWithContext:self atIndex:curTop + i];
                [tuple addReturnValue:[value toObject]];
            }
            retValue = [LSCValue tupleValue:tuple];
        }
        else if (returnCount == 1)
        {
            retValue = [LSCValue valueWithContext:self atIndex:-1];
        }
    }
    else
    {
        //调用失败
        returnCount = 1;
        LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
        NSString *errMessage = [value toString];
        [self raiseExceptionWithMessage:errMessage];
    }
    
    //弹出返回值
    lua_pop(state, returnCount);
    
    if (!retValue)
    {
        retValue = [LSCValue nilValue];
    }
    
    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return retValue;
}

- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
{
    lua_State *state = self.currentSession.state;
    
    LSCValue *resultValue = nil;
    
    int curTop = lua_gettop(state);
    
    lua_getglobal(state, [methodName UTF8String]);
    if (lua_isfunction(state, -1))
    {
        int returnCount = 0;
        
        //如果为function则进行调用
        __weak LSCContext *theContext = self;
        [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
             
             [value pushWithContext:theContext];
             
         }];
        
        if (lua_pcall(state, (int)arguments.count, LUA_MULTRET, 0) == 0)
        {
            //调用成功
            returnCount = lua_gettop(state) - curTop;
            if (returnCount > 1)
            {
                LSCTuple *tuple = [[LSCTuple alloc] init];
                for (int i = 1; i <= returnCount; i++)
                {
                    LSCValue *value = [LSCValue valueWithContext:self atIndex:curTop + i];
                    [tuple addReturnValue:[value toObject]];
                }
                resultValue = [LSCValue tupleValue:tuple];
            }
            else if (returnCount == 1)
            {
                resultValue = [LSCValue valueWithContext:self atIndex:-1];
            }
        }
        else
        {
            //调用失败
            returnCount = 1;
            LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
            NSString *errMessage = [value toString];
            [self raiseExceptionWithMessage:errMessage];
        }
        
        lua_pop(state, returnCount);
    }
    else
    {
        //将变量从栈中移除
        lua_pop(state, 1);
    }
    
    //内存回收
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return resultValue;
}

- (void)registerMethodWithName:(NSString *)methodName
                         block:(LSCFunctionHandler)block
{
    lua_State *state = self.mainSession.state;
    
    if (![self.methodBlocks objectForKey:methodName])
    {
        [self.methodBlocks setObject:block forKey:methodName];
        
        lua_pushlightuserdata(state, (__bridge void *)self);
        lua_pushstring(state, [methodName UTF8String]);
        lua_pushcclosure(state, cfuncRouteHandler, 2);
        lua_setglobal(state, [methodName UTF8String]);
    }
    else
    {
        @throw [NSException
                exceptionWithName:@"Unabled register method"
                reason:@"The method of the specified name already exists!"
                userInfo:nil];
    }
}

- (void)registerModuleWithClass:(Class)moduleClass
{
    if ([moduleClass isSubclassOfClass:[LSCModule class]])
    {
        [moduleClass _regModule:moduleClass context:self];
    }
    else
    {
         [self raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCModule' class!", NSStringFromClass(moduleClass)]];
    }
}

- (void)unregisterModuleWithClass:(Class)moduleClass
{
    if ([moduleClass isSubclassOfClass:[LSCModule class]])
    {
        [moduleClass _unregModule:moduleClass context:self];
    }
    else
    {
        [self raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCModule' class!", NSStringFromClass(moduleClass)]];
    }
}

#pragma mark - Private

- (LSCSession *)makeSessionWithState:(lua_State *)state;
{
    if (self.mainSession.state != state)
    {
        LSCSession *session = [[LSCSession alloc] initWithState:state context:self];
        self.currentSession = session;
        
        return session;
    }
    
    return self.mainSession;
}

- (void)raiseExceptionWithMessage:(NSString *)message
{
    if (self.exceptionHandler)
    {
        self.exceptionHandler (message);
    }
}

static int cfuncRouteHandler(lua_State *state)
{
    //fixed: 修复Lua中在协程调用方法时无法正确解析问题, 使用LSCCallSession解决问题 2017-7-3
    int count = 0;
    
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];

    LSCFunctionHandler handler = context.methodBlocks[methodName];
    if (handler)
    {
        LSCSession *session = [context makeSessionWithState:state];
        NSArray *arguments = [session parseArguments];
        
        LSCValue *retValue = handler(arguments);
        
        if (retValue)
        {
            count = [session setReturnValue:retValue];
        }
    }
    
    return count;
}

/**
 *  设置搜索路径，避免脚本中的require无法找到文件
 *
 *  @param path 搜索路径
 */
- (void)setSearchPath:(NSString *)path
{
    lua_State *state = self.mainSession.state;
    
    lua_getglobal(state, "package");
    lua_getfield(state, -1, "path");
    
    //取出当前路径，并附加新路径
    NSMutableString *curPath =
    [NSMutableString stringWithUTF8String:lua_tostring(state, -1)];
    [curPath appendFormat:@";%@", path];
    
    lua_pop(state, 1);
    lua_pushstring(state, curPath.UTF8String);
    lua_setfield(state, -2, "path");
    lua_pop(state, 1);
}

@end
