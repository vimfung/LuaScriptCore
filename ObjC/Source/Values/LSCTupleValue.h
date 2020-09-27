//
//  LSCTupleValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

/**
 元组值类型
 */
@interface LSCTupleValue : NSObject <LSCValueType>

/**
 长度
 */
@property (nonatomic, readonly) NSInteger count;

/**
 初始化对象

 @param context 上下文对象
 @param stackIndex 栈索引
 @param count 元素数量
 @return 对象实例
 */
- (instancetype)initWithContext:(LSCContext *)context
                     stackIndex:(int)stackIndex
                          count:(NSInteger)count;

/**
 添加对象

 @param value 对象
 */
- (void)addObject:(id)value;

/**
 移除对象

 @param index 对象索引
 */
- (void)removeObjectAtIndex:(NSUInteger)index;

/**
 获取对象

 @param index 对象索引
 @return 对象值
 */
- (id)objectAtIndex:(NSUInteger)index;

@end

NS_ASSUME_NONNULL_END
