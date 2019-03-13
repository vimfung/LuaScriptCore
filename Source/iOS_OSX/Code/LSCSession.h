//
//  LSCCallSession.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/7/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCValue;
@class LSCError;
@class LSCScriptController;

/**
 会话，从Lua调用原生方法时依靠此会话来处理参数和返回值
 */
@interface LSCSession : NSObject

/**
 前一个会话
 */
@property (nonatomic, weak) LSCSession *prevSession;

/**
 最近一次的错误信息
 */
@property (nonatomic, strong, readonly) LSCError *lastError;

/**
 执行脚本配置
 */
@property (nonatomic, strong) LSCScriptController *scriptController;

/**
 解析并获取参数

 @return 参数集合
 */
- (NSArray *)parseArguments;


/**
 解析并获取参数（排除第一个参数）

 @return 参数集合
 */
- (NSArray *)parseArgumentsWithoutTheFirst;

/**
 设置返回值

 @param value 返回值
 @returns 参数数量
 */
- (int)setReturnValue:(LSCValue *)value;

/**
 报告错误信息

 @param message 错误信息
 */
- (void)reportLuaExceptionWithMessage:(NSString *)message;

/**
 清除错误信息
 */
- (void)clearError;

@end
