//
//  LSCThread.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/12/24.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCContext.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCValue;

/**
 线程对象
 */
@interface LSCThread : LSCContext

/**
 上下文对象
 */
@property (nonatomic, weak, readonly) LSCContext *context;

/**
 线程是否运行完成
 */
@property (nonatomic, readonly) BOOL finished;

/**
 初始化

 @param context 上下文对象
 @param handler 线程处理
 @return 线程对象
 */
- (instancetype)initWithContext:(LSCContext *)context
                        handler:(LSCFunctionHandler)handler;

/**
 启动线程
 
 @param arguments 参数列表
 */
- (void)resumeWithArguments:(NSArray<LSCValue *> *)arguments;

/**
 挂起线程
 
 @param result 返回结果
 */
- (void)yieldWithResult:(LSCValue *)result;

@end

NS_ASSUME_NONNULL_END
