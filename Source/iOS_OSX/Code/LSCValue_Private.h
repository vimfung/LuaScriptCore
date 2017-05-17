//
//  LUAValue_Private.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"

@interface LSCValue ()

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

/**
 对象是否被管理，YES 表示Lua对象随LSCValue释放而释放
 */
@property (nonatomic) BOOL hasManagedObject;

/**
 *  获取栈中的某个值
 *
 *  @param context 上下文对象
 *  @param index 栈索引
 *
 *  @return 值对象
 */
+ (LSCValue *)valueWithContext:(LSCContext *)context atIndex:(NSInteger)index;

/**
 管理对象内存，被管理对象与LSCValue有着相同的生存周期，随LSCValue释放而归还内存管理权给Lua

 @param context 上下文对象
 */
- (void)managedObjectWithContext:(LSCContext *)context;

/**
 *  入栈数据
 *
 *  @param context 上下文对象
 */
- (void)pushWithContext:(LSCContext *)context;

@end
