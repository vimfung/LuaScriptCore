//
//  LSCClassInstance.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCObjectClass;
@class LSCValue;

/**
 *  类实例化对象
 */
@interface LSCClassInstance : NSObject

/**
 *  所属本地类型
 */
@property (nonatomic, readonly) Class ownerClass;

/**
 *  lua实例对象对应的本地对象
 */
@property (nonatomic, strong, readonly) LSCObjectClass *nativeObject;

/**
 *  设置字段
 *
 *  @param value 字段值
 *  @param name  字段名称
 */
- (void)setField:(LSCValue *)value forName:(NSString *)name;

/**
 *  获取字段值
 *
 *  @param name 字段名称
 *  
 *  @return 字段值
 */
- (LSCValue *)getFieldForName:(NSString *)name;

@end
