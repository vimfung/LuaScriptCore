//
//  LSCStateWatcher.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/25.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCStateWatcher.h"
#import "LSCException.h"

@interface LSCStateWatcher ()

@property (nonatomic, strong) LSCWatchEventHandler eventHandler;

@end

@implementation LSCStateWatcher

- (instancetype)initWithEvents:(LSCStateWatcherEvents)events
{
    if (self = [super init])
    {
        _events = events;
    }
    
    return self;
}

- (void)onTrigger:(LSCWatchEventHandler)handler
{
    self.eventHandler = handler;
}

- (void)dispatchEvent:(LSCContext *)context
            mainState:(LSCMainState *)mainState
             curState:(LSCState *)curState
{
    if (self.eventHandler)
    {
        self.eventHandler(self, context, mainState, curState);
    }
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

@end
