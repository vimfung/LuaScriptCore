//
//  LSCValueType.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LSCContext;

/**
 值类型
 */
@protocol LSCValueType <NSObject>

/**
 获取实例对象

 @param rawValue 原始值
 @return 实例对象
 */
+ (instancetype)createValue:(_Nullable id)rawValue;

/**
 创建实例对象

 @param context 上下文对象
 @param stackIndex 栈索引
 @return 实例对象
 */
+ (instancetype)createValueWithContext:(LSCContext *)context
                            stackIndex:(int)stackIndex;

/**
 获取原始值

 @return 原始值
 */
- (id)rawValue;

/**
 入栈数据

 @param context 上下文对象
 */
- (void)pushWithContext:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
