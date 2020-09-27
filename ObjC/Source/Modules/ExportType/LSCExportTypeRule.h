//
//  LSCExportTypeRule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

#import "LSCExportFilter.h"

NS_ASSUME_NONNULL_BEGIN

/**
 导出类型规则
 */
@interface LSCExportTypeRule : NSObject

/**
 注册规则，用于替换默认规则

 @param ruleClass 规则类型
 */
+ (void)registerRuleClass:(Class)ruleClass;

/**
 获取默认规则

 @return 规则对象
 */
+ (LSCExportTypeRule *)defaultRule;

/**
 添加导出过滤器
 
 @param filter 过滤器
 */
- (void)addExportFilter:(id<LSCExportFilter>)filter;

/**
 过滤类方法
 
 @param method 方法对象
 @param selector 选择子
 @param cls 类型
 @return YES 表示需要过滤，否则不需要
 */
- (BOOL)filterClassMethod:(const Method)method selector:(SEL)selector class:(Class)cls;

/**
 过滤实例方法

 @param method 方法对象
 @param selector 选择子
 @param cls 类型
 @return YES 表示需要过滤，否则不需要
 */
- (BOOL)filterInstanceMethod:(const Method)method selector:(SEL)selector class:(Class)cls;

/**
 过滤属性

 @param prop 属性对象
 @param cls 类型
 @return YES 表示需要过滤，否则不需要
 */
- (BOOL)filterProperty:(const objc_property_t)prop class:(Class)cls;

/**
 获取类型名称

 @param aClass 类型
 @return 类型名称
 */
- (NSString *)typeNameByClass:(Class)aClass;

/**
 获取原型名称

 @param typeName 类型名称
 @return 原型名称
 */
- (NSString *)prototypeNameByTypeName:(NSString *)typeName;

/**
 构造方法名称

 @return 名称
 */
- (NSString *)constructMethodName;


/**
 方法名称

 @param selector 选择子
 @return 名称
 */
- (NSString *)methodNameBySelector:(SEL)selector;

@end

NS_ASSUME_NONNULL_END
