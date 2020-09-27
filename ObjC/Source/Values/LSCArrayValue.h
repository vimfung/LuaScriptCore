//
//  LSCArrayValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

/**
 数组类型
 */
@interface LSCArrayValue : NSObject <LSCValueType>

/**
 初始化对象

 @param array 数组
 @return 对象实例
 */
- (instancetype)initWithArray:(NSArray *)array;

@end

NS_ASSUME_NONNULL_END
