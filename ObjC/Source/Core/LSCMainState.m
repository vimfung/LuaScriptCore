//
//  LSCMainState.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/10.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCMainState.h"
#import "LSCApiAdapter.h"
#import "LSCState+Private.h"
#import "LSCException.h"
#import "LSCCoroutine.h"
#import "LSCLock.h"

typedef NSMutableDictionary<NSString *, NSMutableArray<LSCState *> *> LSCStateQueueMap;

@interface LSCMainState ()

//锁，用于控制Lua操作同步
@property (nonatomic, strong) LSCLock *locker;

/**
 状态映射表，保留线程ID与状态队列的映射关系，用于快速获取当前State对象。
 注：这里存在一个内存泄露可能，即当线程在外界强制关闭后无法正常释放State导致的内存泄露。
 */
@property (nonatomic, strong) LSCStateQueueMap *stateQueueMap;

@end

@implementation LSCMainState

- (instancetype)initWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    lua_State *state = [apiAdapter createState];
    
    if (state != NULL && (self = [super _initWithRawState:state]))
    {
        _context = context;
        
        //初始化操作锁
        self.locker = [[LSCLock alloc] init];
        
        //放入状态映射表
        [LSCState _pushStatesMapWithState:self];
        
        self.stateQueueMap = [NSMutableDictionary dictionary];
    }
    
    return self;
}

- (LSCState *)currentState
{
    LSCState *state = [[self _getCurrentStateQueue] lastObject];
    if (!state)
    {
        state = self;
    }
    
    return state;
}

- (LSCCoroutine *)currentCoroutine
{
    LSCCoroutine *coroutine = nil;
    NSArray *stateQueue = [self _getCurrentStateQueue];
    for (NSInteger i = stateQueue.count - 1; i >= 0; i--)
    {
        LSCState *state = stateQueue[i];
        if ([state isKindOfClass:[LSCCoroutine class]])
        {
            coroutine = (LSCCoroutine *)state;
            break;
        }
    }
    
    return coroutine;
}

- (void)mount:(LSCState *)state
{
    NSMutableArray *stateQueue = [self _getCurrentStateQueue];
    if (!stateQueue)
    {
        stateQueue = [NSMutableArray array];
        [self.stateQueueMap setObject:stateQueue forKey:[self _getThreadId]];
    }
    
    [stateQueue addObject:state];
}

- (void)unmount
{
    NSMutableArray *stateQueue = [self _getCurrentStateQueue];
    [stateQueue removeLastObject];
}

- (void)lock
{
    [self.locker lock];
}

- (void)unlock
{
    [self.locker unlock];
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

#pragma mark - Private

- (NSMutableArray *)_getCurrentStateQueue
{
    NSString *tid = [self _getThreadId];
    return self.stateQueueMap[tid];
}

- (NSString *)_getThreadId
{
    return [NSString stringWithFormat:@"%p", [NSThread currentThread]];
}

@end
