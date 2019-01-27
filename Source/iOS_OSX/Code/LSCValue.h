//
//  LUAValue.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"

@class LSCContext;
@class LSCFunction;
@class LSCPointer;
@class LSCTuple;
@class LSCExportTypeDescriptor;

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
 *  创建一个指针类型的值对象
 *
 *  @param pointerValue 指针对象
 *
 *  @return 值对象
 */
+ (instancetype)pointerValue:(LSCPointer *)pointerValue;

/**
 创建一个方法引用值对象

 @param functionValue 方法对象

 @return 值对象
 */
+ (instancetype)functionValue:(LSCFunction *)functionValue;

/**
 创建一个元祖值对象

 @param tupleValue 元组对象
 @return 值对象
 */
+ (instancetype)tupleValue:(LSCTuple *)tupleValue;

/**
 创建一个类型值对象

 @param typeDescriptor 类型描述
 @return 值对象
 */
+ (instancetype)typeValue:(LSCExportTypeDescriptor *)typeDescriptor;

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

/**
 *  转换为整型
 *
 *  @return 整型
 */
- (NSInteger)toInteger;

/**
 *  转换为浮点型
 *
 *  @return 浮点型
 */
- (double)toDouble;

/**
 *  转换为布尔型
 *
 *  @return 布尔型
 */
- (BOOL)toBoolean;

/**
 *  转换为二进制数据类型
 *
 *  @return 二进制数据
 */
- (NSData *)toData;

/**
 *  转换为数组
 *
 *  @return 数组
 */
- (NSArray *)toArray;

/**
 *  转换为字典
 *
 *  @return 字典
 */
- (NSDictionary *)toDictionary;

/**
 *  转换为指针
 *
 *  @return 指针值
 */
- (LSCPointer *)toPointer;


/**
 转换为方法

 @return 方法对象
 */
- (LSCFunction *)toFunction;

/**
 转换为元组

 @return 元组
 */
- (LSCTuple *)toTuple;

/**
 转换为类型

 @return 类型描述
 */
- (LSCExportTypeDescriptor *)toType;

/**
 将一个对象放入字典中。注：该方法只有在valueType为LSCValueTypeMap时有效

 @param object 放入字典的对象
 @param keyPath 对应的键名路径，例如："key"、"key1.key2"
 */
- (void)setObject:(id)object forKeyPath:(NSString *)keyPath;

@end
