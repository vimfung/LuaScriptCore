//
//  LSCTuple.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/17.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 元组，仅用于方法返回值
 */
@interface LSCTuple : NSObject

/**
 元素数量
 */
@property (nonatomic, readonly) NSInteger count;

/**
 添加返回值

 @param returnValue 返回值
 */
- (void)addReturnValue:(id)returnValue;

/**
 获取元组中的返回值

 @param index 索引
 @return 返回值
 */
- (id)returnValueForIndex:(NSInteger)index;

@end
