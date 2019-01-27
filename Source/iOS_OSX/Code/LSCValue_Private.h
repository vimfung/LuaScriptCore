//
//  LUAValue_Private.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "LSCSession_Private.h"
#import "LSCTable+Private.h"

@interface LSCValue ()

/**
 *  数值容器
 */
@property (nonatomic, strong) id valueContainer;

/**
 *  数值类型
 */
@property (nonatomic) LSCValueType valueType;

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

/**
 对象是否被管理，YES 表示Lua对象随LSCValue释放而释放
 */
@property (nonatomic) BOOL hasManagedObject;

/**
 初始化

 @param table 表结构
 @return 值对象
 */
+ (instancetype)tableValue:(LSCTable *)table;

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
 从栈中获取临时值，主要用于当Lua中调用方法，一旦经过原生转换后部分类型对象会产生结构上的变化（如：Table），
 因此，使用临时值指的是直接引用栈中的索引，然后直接入栈到方法中，避免经过一层原生方法的转换。

 @param context 上下文对象
 @param index 栈索引
 @return 值对象
 */
+ (LSCValue *)tmpValueWithContext:(LSCContext *)context atIndex:(NSInteger)index;

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


/**
 转换为Table对象

 @return Table对象
 */
- (LSCTable *)toTable;


@end
