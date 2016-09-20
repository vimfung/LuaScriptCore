//
//  LSCModule_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"
#import "lua.h"

@interface LSCModule ()

/**
 *  注册模块
 *
 *  @param state Lua状态机
 */
- (void)_regWithState:(lua_State *)state;

/**
 *  反注册模块
 *
 *  @param state Lua状态机
 */
- (void)_unregWithState:(lua_State *)statej;

@end
