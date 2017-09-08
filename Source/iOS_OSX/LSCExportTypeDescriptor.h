//
//  LSCExportTypeDescriptor.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

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
 父级类型描述器
 */
@property (nonatomic, weak) LSCExportTypeDescriptor *parentTypeDescriptor;

/**
 是否为指定类型的子类型

 @param typeDescriptor 需要检测的父类型
 @return YES 表示为指定类型的子类，否则不是
 */
- (BOOL)subtypeOfType:(LSCExportTypeDescriptor *)typeDescriptor;

@end
