//
//  LSCClassDescription.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCContext;
@class LSCInstance;
@class LSCFunctionValue;

/**
 类型描述，Lua中类型的描述对象
 Lua中并没有面向对象的特性，这里的类型定义是根据table特性虚拟出来的类型
 */
@interface LSCTypeDescription : NSObject

/**
 父级类型描述
 */
@property (nonatomic, weak, readonly) LSCTypeDescription *parentTypeDescription;

/**
 对应的原生类型，如果非直接导出类型，则此值为nil
 */
@property (nonatomic, readonly) Class nativeType;

/**
 类型名称
 */
@property (nonatomic, copy, readonly) NSString *typeName;

/**
 原型名称
 */
@property (nonatomic, copy, readonly) NSString *prototypeName;

/**
 获取导出类型对应的类型描述

 @param aClass 类型
 @return 类型描述
 */
+ (LSCTypeDescription *)typeDescriptionByClass:(Class)aClass;


/**
 获取对象的基类型描述

 @return 类型描述
 */
+ (LSCTypeDescription *)objectTypeDescription;


/**
 调用类方法

 @param name 方法名称
 @param arguments 参数
 @param context 上下文
 @return 返回值
 */
- (id<LSCValueType>)callMethodWithName:(NSString *)name
                             arguments:(NSArray<id<LSCValueType>> *)arguments
                               context:(LSCContext *)context;


/**
 调用实例方法

 @param instance 实例对象
 @param name 方法名称
 @param arguments 参数
 @param context 上下文
 @return 返回值
 */
- (id<LSCValueType>)callMethodWithInstance:(LSCInstance *)instance
                                      name:(NSString *)name
                                 arguments:(NSArray<id<LSCValueType>> *)arguments
                                   context:(LSCContext *)context;

/**
 注册属性

 @param name 属性名称
 @param getterFunc 获取方法
 @param setterFunc 设置方法
 @param context 上下文
 */
- (void)registerPropertyWithName:(NSString *)name
                      getterFunc:(LSCFunctionValue *)getterFunc
                      setterFunc:(LSCFunctionValue *)setterFunc
                         context:(LSCContext *)context;


/**
 获取属性值

 @param instance 实例对象
 @param name 属性名称
 @param context 上下文
 @return 返回值
 */
- (id<LSCValueType>)getPropertyWithInstance:(LSCInstance *)instance
                                       name:(NSString *)name
                                    context:(LSCContext *)context;


/**
 设置属性值

 @param instance 实例对象
 @param name 属性名称
 @param value 属性值
 @param context 上下文
 */
- (void)setPropertyWithInstance:(LSCInstance *)instance
                           name:(NSString *)name
                          value:(id<LSCValueType>)value
                        context:(LSCContext *)context;


/**
 是否存在指定名称属性

 @param name 属性名称
 @return YES 存在，否则不存在
 */
- (BOOL)existsPropertyeWithName:(NSString *)name;

/**
 构建对象

 @param instanceId 实例Id
 @param arguments 构造参数列表
 @param context 上下文
 @return 对象实例
 */
- (LSCInstance *)constructInstanceWithInstanceId:(NSString *)instanceId
                                       arguments:(NSArray<id<LSCValueType>> *)arguments
                                         context:(LSCContext *)context;

/**
 创建该类型的子类

 @param typeName 类型名称
 @return 子类型
 */
- (LSCTypeDescription *)subtypeWithName:(NSString *)typeName;

/**
 是否为指定类型的子类型
 
 @param typeDescription 需要检测的父类型
 @return YES 表示为指定类型的子类，否则不是
 */
- (BOOL)subtypeOfType:(LSCTypeDescription *)typeDescription;


@end

NS_ASSUME_NONNULL_END
