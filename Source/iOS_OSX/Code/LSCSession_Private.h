//
//  LSCCallSession_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/7/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCSession.h"
#import "LSCContext.h"
#import "lauxlib.h"
#import "lua.h"
#import "luaconf.h"
#import "lualib.h"

@interface LSCSession ()

/**
 *  Lua状态
 */
@property (nonatomic, readonly) lua_State *state;

/**
 上下文对象
 */
@property (nonatomic, weak, readonly) LSCContext *context;

/**
 初始化

 @param state 状态
 @return 调用会话
 */
- (instancetype)initWithState:(lua_State *)state
                      context:(LSCContext *)context
                  lightweight:(BOOL)lightweight;

@end
