//
//  LUAValue.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, LSCValueType)
{
    LSCValueTypeNil = 0,
    LSCValueTypeNumber = 1,
    LSCValueTypeBoolean = 2,
    LSCValueTypeString = 3,
    LSCValueTypeTable = 4,
    LSCValueTypeFunction = 5,
    LSCValueTypeThread = 6,
    LSCValueTypeUserData = 7,
    LSCValueTypeInteger = 8,
    LSCValueTypeData = 9
};

/**
 *  Lua的值封装
 */
@interface LSCValue : NSObject

/**
 *  数值类型
 */
@property (nonatomic, readonly) LSCValueType valueType;

/**
 *  创建一个空值对象
 *
 *  @return 值对象
 */
+ (instancetype)nilValue;

/**
 *  创建一个数值对象
 *
 *  @param numberValue 数值
 *
 *  @return 值对象
 */
+ (instancetype)numberValue:(NSNumber *)numberValue;

/**
 *  创建一个布尔值对象
 *
 *  @param boolValue 布尔值
 *
 *  @return 值对象
 */
+ (instancetype)booleanValue:(BOOL)boolValue;

/**
 *  创建一个字符串值对象
 *
 *  @param stringValue 字符串
 *
 *  @return 值对象
 */
+ (instancetype)stringValue:(NSString *)stringValue;

/**
 *  创建一个整型值对象
 *
 *  @param integerValue 整型
 *
 *  @return 值对象
 */
+ (instancetype)integerValue:(NSInteger)integerValue;

/**
 *  创建一个数组对象
 *
 *  @param arrayValue 数组
 *
 *  @return 值对象
 */
+ (instancetype)arrayValue:(NSArray *)arrayValue;

/**
 *  创建一个字典对象
 *
 *  @param dictionaryValue 字典
 *
 *  @return 值对象
 */
+ (instancetype)dictionaryValue:(NSDictionary *)dictionaryValue;

/**
 *  创建一个数据对象
 *
 *  @param dataValue 数据
 *
 *  @return 值对象
 */
+ (instancetype)dataValue:(NSData *)dataValue;

/**
 *  创建一个任意类型的值对象
 *
 *  @param objectValue 对象
 *
 *  @return 值对象
 */
+ (instancetype)objectValue:(id)objectValue;

/**
 *  转换为对象
 *
 *  @return 对象
 */
- (id)toObject;

/**
 *  转换为字符串
 *
 *  @return 字符串
 */
- (NSString *)toString;

/**
 *  转换为数值
 *
 *  @return 数值
 */
- (NSNumber *)toNumber;

@end
