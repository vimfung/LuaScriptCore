//
//  NSNumber+LSC.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSNumber (LSC)

/**
 是否为布尔值Number对象
 */
@property (nonatomic, readonly) BOOL isBoolNumber;

@end

NS_ASSUME_NONNULL_END
