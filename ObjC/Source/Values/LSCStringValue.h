//
//  LSCStringValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN


/**
 字符串数据类型
 */
@interface LSCStringValue : NSObject <LSCValueType>

/**
 原始数据
 */
@property (nonatomic, strong, readonly) NSData *rawData;

/**
 初始化对象

 @param stringValue 字符串
 @return 对象实例
 */
- (instancetype)initWithString:(NSString *)stringValue;


/**
 初始化对象

 @param dataValue 数据对象
 @return 对象实例
 */
- (instancetype)initWithData:(NSData *)dataValue;

@end

NS_ASSUME_NONNULL_END
