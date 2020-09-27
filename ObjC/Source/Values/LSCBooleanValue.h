//
//  LSCBooleanValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

/**
 布尔值类型
 */
@interface LSCBooleanValue : NSObject <LSCValueType>

/**
 初始化对象

 @param boolNumber 布尔值
 @return 对象实例
 */
- (instancetype)initWithBoolNumber:(NSNumber *)boolNumber;

@end

NS_ASSUME_NONNULL_END
