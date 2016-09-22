//
//  LSCModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"

/**
 *  Lua模块，用于扩展时使用。
 *
 *  注：通过定义属性和方法可以直接在lua中使用，对于不需要导出的属性或方法请使用下划线作为名字的第一个字符。
 *     类的初始化函数允许被重写，但不允许多态，只能有一个不带参数的初始化方法。
 */
@interface LSCModule : NSObject

/**
 *  版本
 */
+ (NSString *)version;

@end
