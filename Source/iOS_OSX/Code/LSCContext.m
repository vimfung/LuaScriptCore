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
#import "LSCSession_Private.h"
#import "LSCSession_Private.h"
#import "LSCTuple.h"
#import "LSCCoroutine+Private.h"
#import "LSCError.h"
#import "LSCConfig.h"
#import <objc/runtime.h>

/**
 捕获Lua异常处理器名称
 */
static NSString *const LSCCacheLuaExceptionHandlerName = @"__catchExcepitonHandler";

@interface LSCContext ()

/**
 是否需要回收内存
 */
@property (nonatomic) BOOL needGC;

@end

@implementation LSCContext

- (instancetype)initWithConfig:(LSCConfig *)config
{
    if (self = [super init])
    {
        _config = config;
        self.sessionMap = [NSMutableDictionary dictionary];
        self.methodBlocks = [NSMutableDictionary dictionary];
        
        self.optQueue = [[LSCOperationQueue alloc] init];
        [self.optQueue performAction:^{
            
            lua_State *state = [LSCEngineAdapter newState];
            
            [LSCEngineAdapter gc:state what:LSCGCTypeStop data:0];
            
            //加载标准库
            [LSCEngineAdapter openLibs:state];
            
            [LSCEngineAdapter gc:state what:LSCGCTypeRestart data:0];
            
            //创建主会话
            self.mainSession = [[LSCSession alloc] initWithState:state context:self lightweight:NO];
            
        }];
        
        //设置搜索路径
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        [self addSearchPath:[resourcePath stringByAppendingString:@"/"]];
        
        //初始化数据交换器
        self.dataExchanger = [[LSCDataExchanger alloc] initWithContext:self];
        
        //初始化类型导出器
        self.exportsTypeManager = [[LSCExportsTypeManager alloc] initWithContext:self];
        
        //注册错误捕获方法
        __weak typeof(self) weakSelf = self;
        [self registerMethodWithName:LSCCacheLuaExceptionHandlerName block:^LSCValue *(NSArray<LSCValue *> *arguments) {
            
            if (arguments.count > 0)
            {
                [weakSelf outputExceptionMessage:[arguments[0] toString]];
            }
            return nil;
            
        }];
    }
    
    return self;
}

- (instancetype)init
{
    return [self initWithConfig:[LSCConfig defaultConfig]];
}

- (void)dealloc
{
    //由于LSCSession在销毁前会进行一次GC，但是在该情况下lua_State已经被close。
    //因此，解决方法是保留state对象，然后先销毁session，在进行close
    lua_State *state = self.mainSession.state;
    self.mainSession = nil;

    [self.optQueue performAction:^{

        [LSCEngineAdapter close:state];
        
    }];
    
}

- (LSCSession *)currentSession
{
    NSString *tid = [NSString stringWithFormat:@"%p", [NSThread currentThread]];
    LSCSession *session = self.sessionMap[tid];
    
    if (session)
    {
        return session;
    }
    
    return _mainSession;
}

- (void)raiseExceptionWithMessage:(NSString *)message
{
    LSCError *error = [[LSCError alloc] initWithSession:self.currentSession message:message];
    [self raiseExceptionWithError:error];
}

- (void)onException:(LSCExceptionHandler)handler
{
    self.exceptionHandler = handler;
}

- (void)addSearchPath:(NSString *)path
{
    NSMutableString *fullPath = [NSMutableString stringWithString:path];
    
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
    
    [self setSearchPath:fullPath];
}

- (void)setGlobalWithValue:(LSCValue *)value forName:(NSString *)name
{
    [value pushWithContext:self];
    
    [self.optQueue performAction:^{
        [LSCEngineAdapter setGlobal:self.currentSession.state name:name.UTF8String];
    }];
}

- (LSCValue *)getGlobalForName:(NSString *)name
{
    [self.optQueue performAction:^{
        [LSCEngineAdapter getGlobal:self.currentSession.state name:name.UTF8String];
    }];
    
    LSCValue *retValue = [LSCValue valueWithContext:self atIndex:-1];
    return retValue;
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
    return [self evalScriptFromString:string scriptController:nil];
}

- (LSCValue *)evalScriptFromString:(NSString *)string
                  scriptController:(LSCScriptController *)scriptController
{
    __block LSCValue *returnValue = nil;
    LSCOperationQueue *queue = self.optQueue;
    [queue performAction:^{
        
        LSCSession *session = self.currentSession;
        lua_State *state = session.state;
        
        session.scriptController = scriptController;
        
        int errFuncIndex = [self catchLuaExceptionWithState:state queue:queue];
        int curTop = [LSCEngineAdapter getTop:state];
        int returnCount = 0;
        
        [LSCEngineAdapter loadString:state string:string.UTF8String];
        if ([LSCEngineAdapter pCall:state nargs:0 nresults:LUA_MULTRET errfunc:errFuncIndex] == 0)
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
            returnCount = [LSCEngineAdapter getTop:state] - curTop;
        }
        
        //弹出返回值
        [LSCEngineAdapter pop:state count:returnCount];
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
        if (!returnValue)
        {
            returnValue = [LSCValue nilValue];
        }
        
        //回收内存
        [self gc];
        
        session.scriptController = nil;
        
    }];
    
    return returnValue;
}

- (LSCValue *)evalScriptFromFile:(NSString *)path
{
    return [self evalScriptFromFile:path scriptController:nil];
}

- (LSCValue *)evalScriptFromFile:(NSString *)path
                scriptController:(LSCScriptController *)scriptController
{
    __block LSCValue *retValue = nil;
    
    LSCOperationQueue *queue = self.optQueue;
    [queue performAction:^{
        
        NSString *scriptFilePath = path;
        if (!scriptFilePath)
        {
            NSString *errMessage = @"Lua file path is empty!";
            [self outputExceptionMessage:errMessage];
            
            return;
        }
        
        if (![scriptFilePath hasPrefix:@"/"])
        {
            //应用包内路径
            scriptFilePath = [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] resourcePath], scriptFilePath];
        }
        
        LSCSession *session = self.currentSession;
        lua_State *state = session.state;
        
        session.scriptController = scriptController;
        
        int errFuncIndex = [self catchLuaExceptionWithState:state queue:queue];
        int curTop = [LSCEngineAdapter getTop:state];
        int returnCount = 0;
        
        [LSCEngineAdapter loadFile:state path:scriptFilePath.UTF8String];
        if ([LSCEngineAdapter pCall:state nargs:0 nresults:LUA_MULTRET errfunc:errFuncIndex] == 0)
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
            returnCount = [LSCEngineAdapter getTop:state] - curTop;
        }
        
        //弹出返回值
        [LSCEngineAdapter pop:state count:returnCount];
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
        if (!retValue)
        {
            retValue = [LSCValue nilValue];
        }
        
        //回收内存
        [self gc];
        
        session.scriptController = nil;
        
    }];
    
    return retValue;
}

- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
{
    return [self callMethodWithName:methodName arguments:arguments scriptController:nil];
}

- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
                scriptController:(LSCScriptController *)scriptController
{
    LSCOperationQueue *queue = self.optQueue;
    __block LSCValue *resultValue = nil;
    [queue performAction:^{
        
        LSCSession *session = self.currentSession;
        lua_State *state = session.state;
        
        session.scriptController = scriptController;
        
        int errFuncIndex = [self catchLuaExceptionWithState:state queue:queue];
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
            
            if ([LSCEngineAdapter pCall:state nargs:(int)arguments.count nresults:LUA_MULTRET errfunc:errFuncIndex] == 0)
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
                returnCount = [LSCEngineAdapter getTop:state] - curTop;
            }
            
            [LSCEngineAdapter pop:state count:returnCount];
        }
        else
        {
            //将变量从栈中移除
            [LSCEngineAdapter pop:state count:1];
        }
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
        //内存回收
        [self gc];
        
        session.scriptController = nil;
        
    }];
    
    return resultValue;
}

- (void)registerMethodWithName:(NSString *)methodName
                         block:(LSCFunctionHandler)block
{
    lua_State *state = self.currentSession.state;
    if (![self.methodBlocks objectForKey:methodName])
    {
        [self.optQueue performAction:^{
            
            [self.methodBlocks setObject:block forKey:methodName];
            
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushString:methodName.UTF8String state:state];
            [LSCEngineAdapter pushCClosure:cfuncRouteHandler n:2 state:state];
            [LSCEngineAdapter setGlobal:state name:methodName.UTF8String];
        }];
    }
    else
    {
        @throw [NSException
                exceptionWithName:@"Unabled register method"
                reason:@"The method of the specified name already exists!"
                userInfo:nil];
    }
    
}

- (void)runThreadWithFunction:(LSCFunction *)function
                    arguments:(NSArray<LSCValue *> *)arguments
{
    [self runThreadWithFunction:function arguments:arguments scriptController:nil];
}

- (void)runThreadWithFunction:(LSCFunction *)function
                    arguments:(NSArray<LSCValue *> *)arguments
             scriptController:(LSCScriptController *)scriptController
{
    //创建一个协程状态
    __weak typeof(self) theContext = self;
    LSCCoroutine *coroutine = [[LSCCoroutine alloc] initWithContext:theContext];
    
    dispatch_queue_t queue = dispatch_queue_create("ThreadQueue", DISPATCH_QUEUE_SERIAL);
    dispatch_async(queue, ^{
        
        lua_State *state = coroutine.state;
        
        coroutine.scriptController = scriptController;
        
        //获取捕获错误方法索引
        int errFuncIndex = 0;
        [LSCEngineAdapter getGlobal:state name:LSCCacheLuaExceptionHandlerName.UTF8String];
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            errFuncIndex = [LSCEngineAdapter getTop:state];
        }
        else
        {
            [LSCEngineAdapter pop:state count:1];
        }
        
        int top = [LSCEngineAdapter getTop:state];
        [theContext.dataExchanger getLuaObject:function state:state queue:nil];
        
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            int returnCount = 0;
            
            [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
                
                [theContext.dataExchanger pushStackWithObject:value coroutine:coroutine];
                
            }];
            
            if ([LSCEngineAdapter pCall:state nargs:(int)arguments.count nresults:LUA_MULTRET errfunc:errFuncIndex] == 0)
            {
                returnCount = [LSCEngineAdapter getTop:state] - top;
            }
            else
            {
                //调用失败
                returnCount = [LSCEngineAdapter getTop:state] - top;
            }
            
            //弹出返回值
            [LSCEngineAdapter pop:state count:returnCount];
        }
        else
        {
            //弹出func
            [LSCEngineAdapter pop:state count:1];
        }
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
        //释放内存
        [theContext gc];
        
        coroutine.scriptController = nil;
        
    });
}

#pragma mark - Private

/**
 抛出异常

 @param error 异常信息
 */
- (void)raiseExceptionWithError:(LSCError *)error
{
    if (error)
    {
        [self.optQueue performAction:^{
            
            [LSCEngineAdapter rawRunProtected:error.session.state
                                         func:raiseLuaException
                                     userdata:(void *)error.message.UTF8String];
            
        }];
    }
    
}

- (LSCSession *)makeSessionWithState:(lua_State *)state
                         lightweight:(BOOL)lightweight
{
    NSString *tid = [NSString stringWithFormat:@"%p", [NSThread currentThread]];
    LSCSession *session = [[LSCSession alloc] initWithState:state context:self lightweight:lightweight];
    
    LSCSession *prevSession = self.sessionMap[tid];
    if (prevSession)
    {
        session.prevSession = prevSession;
    }
    
    [self.sessionMap setObject:session forKey:tid];
    
    return session;
}

- (void)destroySession:(LSCSession *)session
{
    [self raiseExceptionWithError:session.lastError];
    [session clearError];
    
    NSString *tid = [NSString stringWithFormat:@"%p", [NSThread currentThread]];
    if (session.prevSession)
    {
        [self.sessionMap setObject:session.prevSession forKey:tid];
    }
    else
    {
        [self.sessionMap removeObjectForKey:tid];
    }
}

/**
 输出异常消息

 @param message 异常消息
 */
- (void)outputExceptionMessage:(NSString *)message
{
    if (self.exceptionHandler)
    {
        self.exceptionHandler (message);
    }
}

/**
 *  设置搜索路径，避免脚本中的require无法找到文件
 *
 *  @param path 搜索路径
 */
- (void)setSearchPath:(NSString *)path
{
    [self.optQueue performAction:^{
        
        lua_State *state = self.currentSession.state;
        
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
        
    }];
}


/**
 捕获Lua异常

 @return 异常方法在堆栈中的位置
 */
- (int)catchLuaExceptionWithState:(lua_State *)state queue:(LSCOperationQueue *)queue
{
    __block int index = 0;
    [queue performAction:^{
        
        [LSCEngineAdapter getGlobal:state name:LSCCacheLuaExceptionHandlerName.UTF8String];
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            index = [LSCEngineAdapter getTop:state];
            return;
        }
        
        [LSCEngineAdapter pop:state count:1];
    }];
   
    return index;
}

- (void)gc
{
    if (!self.needGC)
    {
        //进行定时内存回收检测
        self.needGC = YES;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_global_queue(0, 0), ^{
            
            [self.optQueue performAction:^{
                [LSCEngineAdapter gc:self.currentSession.state what:LSCGCTypeCollect data:0];
            }];
            
            self.needGC = NO;
        });
    }
}

#pragma mark - c func


/**
 抛出Lua异常

 @param state 状态
 @param ud 异常信息
 */
static void raiseLuaException(lua_State *state, void *ud)
{
    const char *msg = (const char *)ud;
    [LSCEngineAdapter error:state message:msg];
}

/**
 C方法路由处理器

 @param state 状态
 @return 参数数量
 */
static int cfuncRouteHandler(lua_State *state)
{
    //fixed: 修复Lua中在协程调用方法时无法正确解析问题, 使用LSCCallSession解决问题 2017-7-3
    LSCContext *context = (__bridge LSCContext *)[LSCEngineAdapter toPointer:state
                                                                       index:[LSCEngineAdapter upvalueIndex:1]];
    
    int count = 0;

    const char *methodNameCStr = [LSCEngineAdapter toString:state
                                                      index:[LSCEngineAdapter upvalueIndex:2]];
    NSString *methodName = [NSString stringWithUTF8String:methodNameCStr];
    
    LSCFunctionHandler handler = context.methodBlocks[methodName];
    if (handler)
    {
        LSCSession *session = [context makeSessionWithState:state lightweight:NO];
        NSArray *arguments = [session parseArguments];
        
        LSCValue *retValue = handler(arguments);
        
        if (retValue)
        {
            count = [session setReturnValue:retValue];
        }
        
        [context destroySession:session];
    }
    
    return count;
}

@end
