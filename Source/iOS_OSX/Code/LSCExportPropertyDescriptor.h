//
//  LSCExportPropertyDescriptor.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/24.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 导出属性描述
 */
@interface LSCExportPropertyDescriptor : NSObject

/**
 属性名称
 */
@property (nonatomic, copy, readonly) NSString *name;

/**
 Getter方法
 */
@property (nonatomic) SEL getterSelector;

/**
 Setter方法
 */
@property (nonatomic) SEL setterSelector;

/**
 初始化属性描述

 @param name 属性名称
 @return 属性描述对象
 */
- (instancetype)initWithName:(NSString *)name;

@end
