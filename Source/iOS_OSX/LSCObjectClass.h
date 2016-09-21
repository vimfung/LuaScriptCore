//
//  LSCClass.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"
#import "LSCModule.h"

@class LSCClassInstance;

/**
 *  Lua对象基类，为lua提供面向对象的基础，继承该类型用于衍生其他的类型
 */
@interface LSCObjectClass : LSCModule

/**
 *  当前对象实例，该实例对应lua中的实例。当调用本地实例方法时，可以调用此方法获取当前的实例。
 */
+ (LSCClassInstance *)currentInstance;

/**
 *  获取类对象实例的描述
 *
 *  @param instance 对象实例, 注：该变量不能保留，只在该方法内有效。
 *
 *  @return 描述字符串
 */
- (NSString *)_instanceDescription:(LSCClassInstance *)instance;

/**
 *  类对象实例初始化时触发
 *
 *  @param instance 对象实例，注：该变量不能保留，只在该方法内有效。
 */
- (void)_instanceInitialize:(LSCClassInstance *)instance;

/**
 *  类对象实例销毁时触发
 *
 *  @param instance 对象实例，注：该变量不能保留，只能在该方法内有效。
 */
- (void)_instanceUninitialize:(LSCClassInstance *)instance;

@end
