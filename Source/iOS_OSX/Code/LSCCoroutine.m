//
//  LSCThread.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/12/24.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCCoroutine.h"
#import "LSCCoroutine+Private.h"
#import "LSCContext_Private.h"
#import "LSCSession_Private.h"
#import "LSCValue_Private.h"
#import "LSCTuple.h"
#import "LSCScriptController+Private.h"

/**
 执行脚本配置
 */
static NSMutableDictionary<NSString *, LSCCoroutine *> *hookCoroutines = nil;

@implementation LSCCoroutine

- (instancetype)initWithContext:(LSCContext *)context
{
    if (self)
    {
        _context = context;
        self.linkId = [NSString stringWithFormat:@"%p", self];
        
        __weak typeof(self) theCoroutine = self;
        [self.context.optQueue performAction:^{
            
            theCoroutine.state = [LSCEngineAdapter newThread:context.currentSession.state];
            
            int top = [LSCEngineAdapter getTop:context.currentSession.state];
            
            //设置Lua对象到_vars_表中
            [context.dataExchanger setLubObjectByStackIndex:top objectId:theCoroutine.linkId];
            //进行引用
            [context.dataExchanger retainLuaObject:theCoroutine];
            
            //将线程状态出栈，让原生层生命周期进行控制
            [LSCEngineAdapter pop:context.currentSession.state count:1];
            
        }];
    }
    
    return self;
}

- (void)dealloc
{
    self.scriptController = nil;
    [self.context.dataExchanger releaseLuaObject:self];
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
        [hookCoroutines removeObjectForKey:key];
        
        [LSCEngineAdapter setHook:_state hook:hookLineFunc mask:0 count:0];
        
        return;
    }
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        hookCoroutines = [NSMutableDictionary dictionary];
    });
    
    NSString *key = [NSString stringWithFormat:@"%p", _state];
    LSCCoroutine *oldCoroutine = hookCoroutines[key];
    if (oldCoroutine)
    {
        oldCoroutine.scriptController = nil;
    }
    
    _scriptController = scriptController;
    [hookCoroutines setObject:self forKey:key];
    
    [LSCEngineAdapter setHook:_state hook:hookLineFunc mask:LUA_MASKLINE count:0];
}

#pragma mark - lua hook method

static void hookLineFunc(lua_State *state, lua_Debug *ar)
{
    NSString *key = [NSString stringWithFormat:@"%p", state];
    LSCCoroutine *coroutine = hookCoroutines[key];
    LSCScriptController *scriptController = coroutine.scriptController;
    
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
