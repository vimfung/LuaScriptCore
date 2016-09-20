//
//  LSCModule_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"
#import "LSCContext.h"
#import "lua.h"

@interface LSCModule ()

/**
 *  获取模块名称
 *
 *  @return 模块名称
 */
+ (NSString *)_moduleName;

/**
 *  注册模块
 *
 *  @param state Lua状态机
 *  @param moduleName   模块名称
 */
- (void)_regWithContext:(LSCContext *)context moduleName:(NSString *)moduleName;

/**
 *  反注册模块
 *
 *  @param state Lua状态机
 *  @param moduleName 模块名称
 */
- (void)_unregWithContext:(LSCContext *)context moduleName:(NSString *)moduleName;

/**
 *  获取Lua方法名称，需要过滤冒号后面所有内容以及带With、By、At等
 *
 *  @param name 原始方法
 *
 *  @return 方法
 */
- (NSString *)_getLuaMethodNameWithName:(NSString *)name;

@end
