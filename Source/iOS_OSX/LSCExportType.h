//
//  LSCExportModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/5.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 导出模块协议，实现此协议可以将模块导出到Lua层使用
 */
@protocol LSCExportType <NSObject>

@optional

/**
 获取模块名称

 @return 模块名称
 */
+ (NSString *)typeName;

/**
 被排除的类方法，被排除的方法无法导出到Lua中

 @return 方法列表
 */
+ (NSArray<NSString *> *)excludeExportClassMethods;

/**
 被排除的实例方法，被排除的方法无法导出到Lua中

 @return 方法列表
 */
+ (NSArray<NSString *> *)excludeExportInstanceMethods;

@end
