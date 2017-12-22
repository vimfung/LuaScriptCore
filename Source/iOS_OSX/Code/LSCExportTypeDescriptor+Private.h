//
//  LSCExportTypeDescriptor+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/12/13.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportTypeDescriptor.h"

@class LSCValue;

@interface LSCExportTypeDescriptor ()

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
 方法映射表
 */
@property (nonatomic, strong) NSMutableDictionary<NSString *,LSCExportMethodDescriptor *> *methodsMapping;

/**
 原生类型属性Selector名称集合，主要用于记录当前类型的所有属性Selector名称。
 解决在导出子类时，如果子类重载父类属性的getter／setter，导出管理器无法检测重载的是方法还是属性，
 因此借助该属性来辅助判断。
 */
@property (nonatomic, strong) NSSet<NSString *> *propertySelectorNames;

/**
 调用方法
 
 @param instance 实例对象
 @param invocation 调用器
 @param arguments 参数
 
 @return 返回值
 */
- (LSCValue *)_invokeMethodWithInstance:(id)instance
                             invocation:(NSInvocation *)invocation
                              arguments:(NSArray *)arguments;

@end
