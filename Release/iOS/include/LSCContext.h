//
//  LUAContext.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCValue;

/**
 *  异常处理器
 *
 *  @param message 异常信息
 */
typedef void (^LSCExceptionHandler) (NSString *message);

/**
 *  方法处理器
 *
 *  @param arguments 参数列表
 *
 *  @return 返回值
 */
typedef LSCValue* (^LSCFunctionHandler) (NSArray *arguments);

/**
 *  Lua上下文对象
 */
@interface LSCContext : NSObject

/**
 *  发生异常时触发
 *
 *  @param handler 事件处理器
 */
- (void)onException:(LSCExceptionHandler)handler;

/**
 *  解析脚本
 *
 *  @param string 脚本字符串
 *
 *  @return 返回值，如果无返回值则为nil
 */
- (LSCValue *)evalScriptFromString:(NSString *)string;

/**
 *  解析脚本
 *
 *  @param path 脚本路径
 *
 *  @return 返回值，如果无返回值则为nil
 */
- (LSCValue *)evalScriptFromFile:(NSString *)path;

/**
 *  调用方法
 *
 *  @param methodName 方法名称
 *  @param arguments  参数
 *
 *  @return 返回值
 */
- (LSCValue *)callMethodWithName:(NSString *)methodName arguments:(NSArray *)arguments;

/**
 *  注册方法
 *
 *  @param methodName 方法名称
 *  @param block      处理过程
 */
- (void)registerMethodWithName:(NSString *)methodName block:(LSCFunctionHandler)block;

@end
