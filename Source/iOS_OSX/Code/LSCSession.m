//
//  LSCCallSession.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/7/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCSession.h"
#import "LSCSession_Private.h"
#import "LSCValue_Private.h"
#import "LSCTuple_Private.h"
#import "LSCContext_Private.h"
#import "LSCEngineAdapter.h"
#import "LSCScriptController+Private.h"
#import "LSCError.h"

/**
 执行脚本配置
 */
static NSMutableDictionary<NSString *, LSCSession *> *hookSessions = nil;

@interface LSCSession ()


/**
 轻量级标识，YES 表示Session销毁后不需要执行内存回收
 */
@property (nonatomic) BOOL lightweight;

@end

@implementation LSCSession

- (instancetype)initWithState:(lua_State *)state context:(LSCContext *)context lightweight:(BOOL)lightweight
{
    if (self = [super init])
    {
        _state = state;
        _context = context;
        self.lightweight = lightweight;
    }
    
    return self;
}

- (void)dealloc
{
    self.scriptController = nil;
    
    if (!self.lightweight)
    {
        //释放内存
        [self.context gc];
    }
}

- (NSArray *)parseArguments
{
    return [self _parseArgumentsFromIndex:1];
}

- (NSArray *)parseArgumentsWithoutTheFirst
{
    return [self _parseArgumentsFromIndex:2];
}

- (int)setReturnValue:(LSCValue *)value
{
    int count = 0;
    if (value)
    {
        if (value.valueType == LSCValueTypeTuple)
        {
            count = (int)[value toTuple].count;
        }
        else
        {
            count = 1;
        }
        
        [value pushWithContext:self.context];
    }
    else
    {
        [self.context.optQueue performAction:^{
            [LSCEngineAdapter pushNil:self.state];
        }];
    }
    
    return count;
}

- (void)setScriptController:(LSCScriptController *)scriptController
{
    if (!scriptController)
    {
        if (!_scriptController)
        {
            return;
        }
        
        if (_scriptController)
        {
            //重置标识
            _scriptController.isForceExit = NO;
            _scriptController.startTime = 0;
            _scriptController = nil;
        }
        
        NSString *key = [NSString stringWithFormat:@"%p", _state];
        [hookSessions removeObjectForKey:key];
        
        [_context.optQueue performAction:^{
            [LSCEngineAdapter setHook:self -> _state hook:hookLineFunc mask:0 count:0];
        }];
        
        return;
    }
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        hookSessions = [NSMutableDictionary dictionary];
    });
    
    NSString *key = [NSString stringWithFormat:@"%p", _state];
    LSCSession *oldSession = hookSessions[key];
    if (oldSession)
    {
        oldSession.scriptController = nil;
    }
    
    _scriptController = scriptController;
    [hookSessions setObject:self forKey:key];
    
    [_context.optQueue performAction:^{
        [LSCEngineAdapter setHook:self -> _state hook:hookLineFunc mask:LUA_MASKLINE count:0];
    }];
}

#pragma mark - Private


/**
 解析参数

 @param index 开始索引
 @return 参数集合
 */
- (NSArray *)_parseArgumentsFromIndex:(int)index
{
    
    __block int top = 0;
    
    [self.context.optQueue performAction:^{
        top = [LSCEngineAdapter getTop:self.state];
    }];
    
    if (top >= index)
    {
        NSMutableArray *arguments = [NSMutableArray array];
        for (int i = index; i <= top; i++)
        {
            LSCValue *value = [LSCValue valueWithContext:self.context atIndex:i];
            [arguments addObject:value];
        }
        
        return arguments;
    }
    
    return nil;
}

- (void)reportLuaExceptionWithMessage:(NSString *)message
{
    _lastError = [[LSCError alloc] initWithSession:self message:message];
}

- (void)clearError
{
    _lastError = nil;
}

#pragma mark - lua hook method

static void hookLineFunc(lua_State *state, lua_Debug *ar)
{
    NSString *key = [NSString stringWithFormat:@"%p", state];
    LSCSession *session = hookSessions[key];
    LSCScriptController *scriptController = session.scriptController;
    
    if (scriptController.isForceExit)
    {
        [LSCEngineAdapter error:state message:"script exit..."];
    }
    else if (scriptController.timeout > 0)
    {
        if (scriptController.startTime < 1)
        {
            scriptController.startTime = CFAbsoluteTimeGetCurrent();
        }
        
        if (CFAbsoluteTimeGetCurrent() - scriptController.startTime > scriptController.timeout)
        {
            [LSCEngineAdapter error:state message:"script exit..."];
        }
    }
    
}

@end
