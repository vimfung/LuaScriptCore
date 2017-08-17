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
        
        lua_State *state = [LSCEngineAdapter newState];
        
        [LSCEngineAdapter gc:state what:LSCGCTypeStop data:0];

        //加载标准库
        [LSCEngineAdapter openLibs:state];
        
        [LSCEngineAdapter gc:state what:LSCGCTypeRestart data:0];
        
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
    [LSCEngineAdapter close:self.mainSession.state];
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
    NSMutableString *fullPath = [NSMutableString stringWithString:path];
    if (![path hasSuffix:@"/"])
    {
        [fullPath appendString:@"/"];
    }
    [fullPath appendString:@"?.lua"];
    
    [self setSearchPath:fullPath];
}

- (void)setGlobalWithValue:(LSCValue *)value forName:(NSString *)name
{
    [value pushWithContext:self];
    [LSCEngineAdapter setGlobal:self.mainSession.state name:name.UTF8String];
}

- (LSCValue *)getGlobalForName:(NSString *)name
{
    [LSCEngineAdapter getGlobal:self.mainSession.state name:name.UTF8String];
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
    int curTop = [LSCEngineAdapter getTop:state];
    int returnCount = 0;
    
    [LSCEngineAdapter loadString:state string:string.UTF8String];
    if ([LSCEngineAdapter pCall:state nargs:0 nresults:LUA_MULTRET errfunc:0] == 0)
    {
        //调用成功
        returnCount = [LSCEngineAdapter getTop:state] - curTop;
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
    [LSCEngineAdapter pop:state count:returnCount];
    
    if (!returnValue)
    {
        returnValue = [LSCValue nilValue];
    }
    
    //回收内存
    [LSCEngineAdapter gc:state what:LSCGCTypeCollect data:0];
    
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
    int curTop = [LSCEngineAdapter getTop:state];
    int returnCount = 0;
    
    [LSCEngineAdapter loadFile:state path:path.UTF8String];
    if ([LSCEngineAdapter pCall:state nargs:0 nresults:LUA_MULTRET errfunc:0] == 0)
    {
        //调用成功
        returnCount = [LSCEngineAdapter getTop:state] - curTop;
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
    [LSCEngineAdapter pop:state count:returnCount];
    
    if (!retValue)
    {
        retValue = [LSCValue nilValue];
    }
    
    //回收内存
    [LSCEngineAdapter gc:state what:LSCGCTypeCollect data:0];
    
    return retValue;
}

- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
{
    lua_State *state = self.currentSession.state;
    
    LSCValue *resultValue = nil;
    
    int curTop = [LSCEngineAdapter getTop:state];
    
    [LSCEngineAdapter getGlobal:state name:methodName.UTF8String];
    if ([LSCEngineAdapter isFunction:state index:-1])
    {
        int returnCount = 0;
        
        //如果为function则进行调用
        __weak LSCContext *theContext = self;
        [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
             
             [value pushWithContext:theContext];
             
         }];
        
        if ([LSCEngineAdapter pCall:state nargs:(int)arguments.count nresults:LUA_MULTRET errfunc:0] == 0)
        {
            //调用成功
            returnCount = [LSCEngineAdapter getTop:state] - curTop;
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
        
        [LSCEngineAdapter pop:state count:returnCount];
    }
    else
    {
        //将变量从栈中移除
        [LSCEngineAdapter pop:state count:1];
    }
    
    //内存回收
    [LSCEngineAdapter gc:state what:LSCGCTypeCollect data:0];
    
    return resultValue;
}

- (void)registerMethodWithName:(NSString *)methodName
                         block:(LSCFunctionHandler)block
{
    lua_State *state = self.mainSession.state;
    
    if (![self.methodBlocks objectForKey:methodName])
    {
        [self.methodBlocks setObject:block forKey:methodName];
        
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushString:methodName.UTF8String state:state];
        [LSCEngineAdapter pushCClosure:cfuncRouteHandler n:2 state:state];
        [LSCEngineAdapter setGlobal:state name:methodName.UTF8String];
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
    
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    
    const char *methodNameCStr = [LSCEngineAdapter toString:state
                                                      index:[LSCEngineAdapter upvalueIndex:2]];
    NSString *methodName = [NSString stringWithUTF8String:methodNameCStr];

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
    
    [LSCEngineAdapter getGlobal:state name:"package"];
    [LSCEngineAdapter getField:state index:-1 name:"path"];
    
    //取出当前路径，并附加新路径
    NSMutableString *curPath =
    [NSMutableString stringWithUTF8String:lua_tostring(state, -1)];
    [curPath appendFormat:@";%@", path];
    
    [LSCEngineAdapter pop:state count:1];
    [LSCEngineAdapter pushString:curPath.UTF8String state:state];
    [LSCEngineAdapter setField:state index:-2 name:"path"];
    [LSCEngineAdapter pop:state count:1];
}

@end
