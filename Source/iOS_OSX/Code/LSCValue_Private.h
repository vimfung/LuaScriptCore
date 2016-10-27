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
 *  获取栈中的某个值
 *
 *  @param context 上下文对象
 *  @param index 栈索引
 *
 *  @return 值对象
 */
+ (LSCValue *)valueWithContext:(LSCContext *)context atIndex:(NSInteger)index;

/**
 *  入栈数据
 *
 *  @param context 上下文对象
 */
- (void)pushWithContext:(LSCContext *)context;

@end
