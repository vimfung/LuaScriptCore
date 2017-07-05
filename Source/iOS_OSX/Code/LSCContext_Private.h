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

@class LSCSession;

@interface LSCContext ()

/**
 *  Lua解析器
 */
//@property(nonatomic) lua_State *state;

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
 获取当前会话，每次与Lua进行交互都会产生一个会话，该会话在交互结束后销毁.
 借助该会话可以解析Lua传递的参数，并且可以给Lua设置返回值
 */
@property (nonatomic, weak) LSCSession *currentSession;

/**
 获取主会话对象
 */
@property (nonatomic, strong) LSCSession *mainSession;

/**
 抛出异常

 @param message 消息
 */
- (void)raiseExceptionWithMessage:(NSString *)message;

/**
 通过状态设置当前会话

 @param state 状态
 */
- (LSCSession *)makeSessionWithState:(lua_State *)state;

@end
