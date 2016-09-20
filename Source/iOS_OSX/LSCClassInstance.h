//
//  LSCClassInstance.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCClass;

/**
 *  类实例化对象
 */
@interface LSCClassInstance : NSObject

/**
 *  所属类型
 */
@property (nonatomic, strong, readonly) LSCClass *ownerClass;

/**
 *  初始化类实例化对象
 *
 *  @param ownerClass 所属类型
 *
 *  @return 实例对象
 */
- (instancetype)initWithClass:(LSCClass *)ownerClass;

/**
 *  设置字段
 *
 *  @param value 字段值
 *  @param name  字段名称
 */
- (void)setField:(id)value forName:(NSString *)name;

/**
 *  获取字段值
 *
 *  @param name 字段名称
 *  
 *  @return 字段值
 */
- (id)getFieldForName:(NSString *)name;

@end
