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
- (LSCValue *)callMethodWithName:(NSString *)methodName arguments:(NSArray<LSCValue *> *)arguments;

/**
 *  注册方法
 *
 *  @param methodName 方法名称
 *  @param block      处理过程
 */
- (void)registerMethodWithName:(NSString *)methodName block:(LSCFunctionHandler)block;


/**
 *  注册模块
 *
 *  @param moduleClass 模块类型，必须继承于LSCModule类
 */
- (void)registerModuleWithClass:(Class)moduleClass;

/**
 *  注销模块
 *
 *  @param moduleClass 模块类型，必须继承于LSCModule类
 */
- (void)unregisterModuleWithClass:(Class)moduleClass;

@end
