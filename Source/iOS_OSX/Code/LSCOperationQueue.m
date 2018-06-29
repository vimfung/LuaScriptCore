//
//  LSCOperationQueue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/6/28.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCOperationQueue.h"

static char const* OptQueueKey = "ScriptOptQueue";

@interface LSCOperationQueue ()

/**
 队列
 */
@property (nonatomic) dispatch_queue_t queue;

@end

@implementation LSCOperationQueue

- (instancetype)init
{
    if (self = [super init])
    {
        self.queue = dispatch_queue_create("LuaScriptCore-Opt-Queue", DISPATCH_QUEUE_SERIAL);
        dispatch_queue_set_specific(self.queue, OptQueueKey, &OptQueueKey, NULL);
    }
    return self;
}

- (void)performAction:(void (^)(void))block
{
    if (dispatch_get_specific(OptQueueKey))
    {
        block();
    }
    else
    {
        dispatch_sync(self.queue, block);
    }
}

@end
