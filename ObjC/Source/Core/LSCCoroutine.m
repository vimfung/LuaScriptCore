//
//  LSCCoroutine.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/30.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCCoroutine.h"
#import "LSCException.h"
#import "LSCApiAdapter.h"
#import "LSCContext.h"
#import "LSCState+Private.h"
#import "LSCLock.h"

@interface LSCCoroutine ()

/**
 上下文
 */
@property (nonatomic, weak) LSCContext *context;

/**
 执行队列
 */
@property (nonatomic) dispatch_queue_t queue;

/**
 处理方法
 */
@property (nonatomic, strong) LSCFunctionValue *handler;

/**
 线程Id
 */
@property (nonatomic, copy) NSString *threadId;

@end

@implementation LSCCoroutine

- (instancetype)initWithContext:(nonnull LSCContext *)context
                        handler:(nonnull LSCFunctionValue *)handler
                          queue:(nullable dispatch_queue_t)queue
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    lua_State *rawState = NULL;
    NSString *threadId = [apiAdapter createThread:&rawState context:context];
    
    if (rawState != NULL && (self = [super _initWithRawState:rawState]))
    {
        //放入状态映射表
        [LSCState _pushStatesMapWithState:self];
        
        self.threadId = threadId;
        self.context = context;
        if (!queue)
        {
            queue = dispatch_get_main_queue();
        }
        self.queue = queue;
        self.handler = handler;
    }
    
    return self;
    
}

- (void)resumeWithArguments:(nullable NSArray<id<LSCValueType>> *)arguments
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter resumeWithCoroutine:self arguments:arguments];
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

- (void)dealloc
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter closeThreadWithId:self.threadId context:self.context];
}

@end
