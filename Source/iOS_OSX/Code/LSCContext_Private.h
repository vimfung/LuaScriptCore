//
//  LSCContext_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/20.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCContext.h"
#import "lauxlib.h"
#import "lua.h"
#import "luaconf.h"
#import "lualib.h"
#import "LSCDataExchanger.h"

@interface LSCContext ()

/**
 *  Lua解析器
 */
@property(nonatomic) lua_State *state;

/**
 *  异常处理器
 */
@property(nonatomic, strong) LSCExceptionHandler exceptionHandler;

/**
 *  方法处理器集合
 */
@property(nonatomic, strong) NSMutableDictionary *methodBlocks;

/**
 数据交换器
 */
@property (nonatomic, strong) LSCDataExchanger *dataExchanger;


/**
 抛出异常

 @param message 消息
 */
- (void)raiseExceptionWithMessage:(NSString *)message;

@end
