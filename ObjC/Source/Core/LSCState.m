//
//  LuaState.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCState.h"
#import "LSCApiAdapter.h"
#import "LSCWeakReference.h"
#import "LSCState+Private.h"
#import "LSCStateWatcher.h"

@implementation LSCState

+ (instancetype)stateWithRawState:(lua_State *)rawState
{
    NSMutableDictionary *statesMap = [self _statesMap];
    NSString *stateId = [self _stateIdWithRawState:rawState];
    LSCState *state = nil;
    LSCWeakReference *wr = statesMap[stateId];
    if (wr && wr.target)
    {
        state = wr.target;
    }
    
    if (!state)
    {
        //创建新对象
        state = [[LSCState alloc] _initWithRawState:rawState];
        wr = [[LSCWeakReference alloc] initWithTarget:state];
        [statesMap setObject:wr forKey:stateId];
    }
    
    
    return state;
}

- (void)setWatcher:(LSCStateWatcher *)watcher
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    _watcher = watcher;
    if (_watcher)
    {
        //开始观察
        [apiAdapter startWatchEvent:(LSCWatchEvents)watcher.events state:self count:0];
    }
    else
    {
        //结束观察
        [apiAdapter stopWatchEventWithstate:self];
    }
}

#pragma mark - Rewrite

- (void)dealloc
{
    NSString *stateId = [LSCState _stateIdWithRawState:self.rawState];
    [[LSCState _statesMap] removeObjectForKey:stateId];
}

#pragma mark - Private

- (instancetype)_initWithRawState:(lua_State *)rawState;
{
    if (self = [super init])
    {
        self.rawState = rawState;
    }
    
    return self;
}


/**
 放入一个状态到状态映射表

 @param state 状态对象
 */
+ (void)_pushStatesMapWithState:(LSCState *)state
{
    NSString *stateId = [LSCState _stateIdWithRawState:state.rawState];
    LSCWeakReference *wr = [[LSCWeakReference alloc] initWithTarget:state];
    [[LSCState _statesMap] setObject:wr forKey:stateId];
}

/**
 获取状态映射表
 
 @return 映射表
 */
+ (NSMutableDictionary<NSString *, LSCWeakReference *> *)_statesMap
{
    /**
     状态映射表，用于记录生成的lua_State,其中key为lua_State的内存标识。
     */
    static NSMutableDictionary<NSString *, LSCWeakReference *> *statesMap;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        statesMap = [NSMutableDictionary dictionary];
    });
    
    return statesMap;
}

/**
 获取原始状态的内存标识
 
 @param rawState 原始状态
 @return 内存标识
 */
+ (NSString *)_stateIdWithRawState:(lua_State *)rawState
{
    return [NSString stringWithFormat:@"%p", rawState];
}

@end
