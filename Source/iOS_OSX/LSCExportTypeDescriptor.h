//
//  LSCExportTypeDescriptor.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCExportMethodDescriptor;
@class LSCValue;

/**
 导出类型描述器
 */
@interface LSCExportTypeDescriptor : NSObject

/**
 类型名称
 */
@property (nonatomic, copy) NSString *typeName;

/**
 原型类型名称
 */
@property (nonatomic, copy) NSString *prototypeTypeName;

/**
 原生类型
 */
@property (nonatomic) Class nativeType;

/**
 导出类方法
 */
@property (nonatomic, strong) NSDictionary<NSString *, NSArray<LSCExportMethodDescriptor *> *> *classMethods;

/**
 导出实例方法
 */
@property (nonatomic, strong) NSDictionary<NSString *, NSArray<LSCExportMethodDescriptor *> *> *instanceMethods;

/**
 父级类型描述器
 */
@property (nonatomic, weak) LSCExportTypeDescriptor *parentTypeDescriptor;

/**
 是否为指定类型的子类型

 @param typeDescriptor 需要检测的父类型
 @return YES 表示为指定类型的子类，否则不是
 */
- (BOOL)subtypeOfType:(LSCExportTypeDescriptor *)typeDescriptor;

/**
 通过传入参数列表来获取类方法

 @param name 方法名称
 @param arguments 传入参数列表
 @return 类方法描述
 */
- (LSCExportMethodDescriptor *)classMethodWithName:(NSString *)name arguments:(NSArray<LSCValue *> *)arguments;

/**
 通过传入参数列表来获取实例方法

 @param name 方法名称
 @param arguments 传入参数列表
 @return 示例方法描述
 */
- (LSCExportMethodDescriptor *)instanceMethodWithName:(NSString *)name arguments:(NSArray<LSCValue *> *)arguments;

@end
