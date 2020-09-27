//
//  LSCScriptControlWatcher.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/26.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCScriptControlWatcher.h"
#import "LSCContext.h"
#import "LSCMainState.h"
#import "LSCApiAdapter.h"

@interface LSCScriptControlWatcher ()

/**
 脚本开始执行时间
 */
@property (nonatomic) CFAbsoluteTime startTime;

/**
 退出脚本标识
 */
@property (nonatomic) BOOL exitScriptFlag;

/**
 退出事件处理器
 */
@property (nonatomic, strong) LSCScriptExitEventHandler exitEventHandler;

/**
 超时事件处理器
 */
@property (nonatomic, strong) LSCScriptTimeoutEventHandler timeoutEventHandler;

@end

@implementation LSCScriptControlWatcher

- (void)setScriptTimeout:(NSTimeInterval)scriptTimeout
{
    _scriptTimeout = scriptTimeout;
    
    //设置超时后需要重置状态
    [self reset];
}

- (void)onExit:(LSCScriptExitEventHandler)handler
{
    self.exitEventHandler = handler;
}

- (void)onTimeout:(LSCScriptTimeoutEventHandler)handler;
{
    self.timeoutEventHandler = handler;
}

- (void)exitScript
{
    @synchronized (self)
    {
        self.exitScriptFlag = YES;
    }
}

- (void)reset
{
    @synchronized (self)
    {
        self.startTime = CFAbsoluteTimeGetCurrent();
        self.exitScriptFlag = NO;
    }
}

#pragma mark - Rewrite

- (instancetype)init
{
    if (self = [super initWithEvents:LSCStateWatcherEventLine])
    {
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        
        [self reset];
        [self onTrigger:^(LSCStateWatcher * _Nonnull watcher, LSCContext * _Nonnull context, LSCMainState * _Nonnull mainState, LSCState * _Nonnull curState) {
           
            LSCScriptControlWatcher *w = (LSCScriptControlWatcher *)watcher;
            if (w.exitScriptFlag)
            {
                
                if (w.exitEventHandler)
                {
                    w.exitEventHandler(w, context);
                }
                [w reset];
                
                [apiAdapter interruptWithMessage:@"The script is forced to exit" context:context];
            }
            else if (w.scriptTimeout > 0)
            {
                if (CFAbsoluteTimeGetCurrent() - w.startTime > w.scriptTimeout)
                {
                    if (w.timeoutEventHandler)
                    {
                        w.timeoutEventHandler(w, context);
                    }
                    [w reset];
                    
                    [apiAdapter interruptWithMessage:@"Script timeout" context:context];
                }
            }
            
        }];
    }
    return self;
}

@end
