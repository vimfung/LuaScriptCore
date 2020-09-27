//
//  LSCTypeDescription+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCTypeDescription.h"
#import <objc/runtime.h>

@class LSCMethodDescription;
@class LSCPropertyDescription;

typedef NSMutableDictionary<NSString *, NSMutableArray<LSCMethodDescription *> *> LSCMethodMap;
typedef NSMutableDictionary<NSString *, LSCPropertyDescription *> LSCPropertyMap;

NS_ASSUME_NONNULL_BEGIN

@interface LSCTypeDescription ()

/**
 父级类型描述
 */
@property (nonatomic, weak) LSCTypeDescription *parentTypeDescription;

/**
 对应的原生类型，如果非直接导出类型，则此值为nil
 */
@property (nonatomic) Class nativeType;

/**
 类型名称
 */
@property (nonatomic, copy) NSString *typeName;

/**
 原型名称
 */
@property (nonatomic, copy) NSString *prototypeName;

/**
 导出的类方法
 Key - 方法名
 Value - 方法数组，方法可能存在多个版本
 */
@property (nonatomic, strong) LSCMethodMap *classMethods;


/**
 导出的实例方法
 Key - 方法名
 Value - 方法数组，方法可能存在多个版本
 */
@property (nonatomic, strong) LSCMethodMap *instanceMethods;

/**
 导出属性
 */
@property (nonatomic, strong) LSCPropertyMap *properties;

/**
 调用方法

 @param method 方法
 @param invocation 调用器
 @param arguments 参数
 @param context 上下文
 @return 返回值
 */
- (id<LSCValueType>)_callMethod:(Method)method
                     invocation:(NSInvocation *)invocation
                      arguments:(nullable NSArray<id<LSCValueType>> *)arguments
                        context:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
