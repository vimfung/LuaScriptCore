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
 *  注：在模块中定义的类方法会映射到lua中使用，对于不需要导出的属性或方法请使用下划线作为名字的第一个字符。
 */
@interface LSCModule : NSObject

/**
 *  获取模块版本
 *
 *  @return 模块版本
 */
+ (NSString *)version;

/**
 *  获取模块名称
 *
 *  @return 模块名称
 */
+ (NSString *)moduleName;

@end
