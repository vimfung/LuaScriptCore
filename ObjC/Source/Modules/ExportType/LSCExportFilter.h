//
//  LSCExportMethodFilter.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

NS_ASSUME_NONNULL_BEGIN


/**
 导出过滤器
 */
@protocol LSCExportFilter <NSObject>

@optional

/**
 过滤类方法

 @param method 方法对象
 @param selector 选择子
 @param cls 类型
 @return YES 表示需要过滤，否则不过滤
 */
- (BOOL)filterClassMethod:(const Method)method
                 selector:(SEL)selector
                    class:(Class)cls;


/**
 过滤实例方法

 @param method 方法对象
 @param selector 选择子
 @param cls 类型
 @return YES 表示需要过滤，否则不过滤
 */
- (BOOL)filterInstanceMethod:(const Method)method
                    selector:(SEL)selector
                       class:(Class)cls;


/**
 过滤属性

 @param prop 属性对象
 @param cls 类型
 @return YES 表示需要过滤，否则不过滤
 */
- (BOOL)filterProperty:(const objc_property_t)prop
                 class:(Class)cls;

@end

NS_ASSUME_NONNULL_END
