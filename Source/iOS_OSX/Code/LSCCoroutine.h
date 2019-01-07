//
//  LSCThread.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/12/24.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LSCContext;

/**
 线程对象
 */
@interface LSCCoroutine : NSObject

/**
 上下文对象
 */
@property (nonatomic, weak, readonly) LSCContext *context;

/**
 初始化

 @param context 上下文对象
 @param handler 线程处理
 @return 线程对象
 */
- (instancetype)initWithContext:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
