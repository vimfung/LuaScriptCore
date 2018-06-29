//
//  LSCContext_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/20.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCContext.h"
#import "LSCEngineAdapter.h"
#import "LSCDataExchanger.h"
#import "LSCExportsTypeManager.h"
#import "LSCOperationQueue.h"

@class LSCSession;

@interface LSCContext ()

/**
 操作队列
 */
@property (nonatomic, strong) LSCOperationQueue *optQueue;

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
@property (nonatomic, strong) LSCSession *currentSession;

/**
 获取主会话对象
 */
@property (nonatomic, strong) LSCSession *mainSession;

/**
 导出类型管理器
 */
@property (nonatomic, strong) LSCExportsTypeManager *exportsTypeManager;

/**
 通过状态设置当前会话

 @param state 状态
 @param lightweight 轻量级标识
 */
- (LSCSession *)makeSessionWithState:(lua_State *)state
                         lightweight:(BOOL)lightweight;

/**
 销毁会话

 @param session 会话对象
 */
- (void)destroySession:(LSCSession *)session;

/**
 捕获Lua异常
 
 @return 异常方法在堆栈中的位置
 */
- (int)catchLuaException;

/**
 内存回收
 */
- (void)gc;

@end
