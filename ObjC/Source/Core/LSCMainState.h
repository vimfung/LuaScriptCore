//
//  LSCMainState.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/10.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCState.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCCoroutine;
@class LSCContext;

/**
 主状态
 */
@interface LSCMainState : LSCState

/**
 当前状态
 */
@property (nonatomic, readonly) LSCState *currentState;

/**
 当前协程
 */
@property (nonatomic, readonly, nullable) LSCCoroutine *currentCoroutine;

/**
 上下文
 */
@property (nonatomic, weak, readonly) LSCContext *context;


/**
 初始化主状态

 @param context 上下文
 @return 主状态
 */
- (instancetype)initWithContext:(LSCContext *)context;

/**
 挂载当前线程下的State对象
 
 @param state State对象
 */
- (void)mount:(LSCState *)state;

/**
 卸载当前线程下的当前State对象
 */
- (void)unmount;


/**
 锁定
 */
- (void)lock;

/**
 解锁
 */
- (void)unlock;

@end

NS_ASSUME_NONNULL_END
