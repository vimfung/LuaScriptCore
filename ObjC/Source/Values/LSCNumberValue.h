//
//  LSCNumberValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

/**
 数字值类型
 */
@interface LSCNumberValue : NSObject <LSCValueType>


/**
 初始化对象

 @param numberValue 数值
 @return 对象实例
 */
- (instancetype)initWithNumber:(NSNumber *)numberValue;

@end

NS_ASSUME_NONNULL_END
