//
//  LSCCoroutine.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/30.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCState.h"
#import "LSCValueType.h"

@class LSCContext;
@class LSCFunctionValue;
@class LSCCoroutine;

/**
 *  返回事件处理器
 *
 *  @param resultValue 返回值
 */
typedef void (^LSCCoroutineResultHandler) (LSCCoroutine *coroutine, id<LSCValueType> resultValue);

NS_ASSUME_NONNULL_BEGIN

/**
 协程
 */
@interface LSCCoroutine : LSCState


/**
 上下文
 */
@property (nonatomic, weak, readonly) LSCContext *context;

/**
 执行队列
 */
@property (nonatomic, readonly) dispatch_queue_t queue;

/**
 处理方法
 */
@property (nonatomic, strong, readonly) LSCFunctionValue *handler;

/**
 返回事件处理器
 */
@property (nonatomic, strong) LSCCoroutineResultHandler resultHandler;

/**
 初始化对象

 @param context 上下文
 @param handler 处理器
 @param queue 协程在的执行队列，如果为nil，则表示在主线程中执行。
 @return 对象实例
 */
- (instancetype)initWithContext:(nonnull LSCContext *)context
                        handler:(nonnull LSCFunctionValue *)handler
                          queue:(nullable dispatch_queue_t)queue;


/**
 启动协程或者恢复协程运行

 @param arguments 传入参数
 */
- (void)resumeWithArguments:(nullable NSArray<id<LSCValueType>> *)arguments;

@end

NS_ASSUME_NONNULL_END
