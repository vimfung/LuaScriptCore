//
//  LUAContext.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"

@class LSCModule;

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
 *  添加搜索路径，如果执行的lua脚本不是放在默认目录（应用目录）内时，需要添加指定路径，否则会提示无法找到脚本从而运行出错
 *
 *  @param path 路径
 */
- (void)addSearchPath:(NSString *)path;

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

/**
 *  添加模块
 *
 *  @param module 模块
 */
- (void)addModule:(LSCModule *)module;

/**
 *  移除模块
 *
 *  @param module 模块
 */
- (void)removeModule:(LSCModule *)module;

@end
