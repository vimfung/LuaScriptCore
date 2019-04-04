//
//  LUAContext.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"

@class LSCFunction;
@class LSCScriptController;
@class LSCConfig;

/**
 *  Lua上下文对象
 */
@interface LSCContext : NSObject

/**
 配置信息
 */
@property (nonatomic, strong, readonly) LSCConfig *config;


/**
 初始化

 @return 上下文对象
 */
- (instancetype)init;

/**
 初始化

 @param config 配置信息
 @return 上下文对象
 */
- (instancetype)initWithConfig:(LSCConfig *)config;

/**
 抛出异常
 
 @param message 消息
 */
- (void)raiseExceptionWithMessage:(NSString *)message;

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
 设置全局变量

 @param value 变量值
 @param name 名称
 */
- (void)setGlobalWithValue:(LSCValue *)value forName:(NSString *)name;

/**
 获取全局变量

 @param name 变量名称
 @return 变量值
 */
- (LSCValue *)getGlobalForName:(NSString *)name;

/**
 保留Lua层的变量引用，使其不被GC所回收。
 注：判断value能否被保留取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
 即：LSCValue *val1 = [LSCValue objectValue:obj1]与LSCValue *val2 = [LSCValue objectValue:obj1]传入方法中效果相同。
 
 
 @param value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。
 */
- (void)retainValue:(LSCValue *)value;

/**
 释放Lua层的变量引用，使其内存管理权交回Lua。
 注：判断value能否被释放取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
 即：LSCValue *val1 = [LSCValue objectValue:obj1]与LSCValue *val2 = [LSCValue objectValue:obj1]传入方法中效果相同。

 @param value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。
 */
- (void)releaseValue:(LSCValue *)value;

/**
 *  解析脚本
 *
 *  @param string 脚本字符串
 *
 *  @return 返回值
 */
- (LSCValue *)evalScriptFromString:(NSString *)string;

/**
 解析脚本

 @param string 脚本字符串
 @param scriptController 脚本控制器
 @return 返回值
 */
- (LSCValue *)evalScriptFromString:(NSString *)string
                  scriptController:(LSCScriptController *)scriptController;

/**
 *  解析脚本
 *
 *  @param path 脚本路径
 *
 *  @return 返回值
 */
- (LSCValue *)evalScriptFromFile:(NSString *)path;


/**
 解析脚本

 @param path 脚本路径
 @param scriptController 脚本控制器
 @return 返回值
 */
- (LSCValue *)evalScriptFromFile:(NSString *)path
                scriptController:(LSCScriptController *)scriptController;

/**
 *  调用方法
 *
 *  @param methodName 方法名称
 *  @param arguments  参数
 *
 *  @return 返回值
 */
- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments;


/**
 调用方法

 @param methodName 方法名称
 @param arguments 参数
 @param scriptController 脚本控制器
 @return 返回值
 */
- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
                scriptController:(LSCScriptController *)scriptController;

/**
 *  注册方法
 *
 *  @param methodName 方法名称
 *  @param block      处理过程
 */
- (void)registerMethodWithName:(NSString *)methodName block:(LSCFunctionHandler)block;


/**
 将指定方法放入线程中执行

 @param function 方法对象
 @param arguments 参数
 */
- (void)runThreadWithFunction:(LSCFunction *)function
                    arguments:(NSArray<LSCValue *> *)arguments;


/**
 将指定方法放入线程中执行

 @param function 方法对象
 @param arguments 参数
 @param scriptController 脚本控制器
 */
- (void)runThreadWithFunction:(LSCFunction *)function
                    arguments:(NSArray<LSCValue *> *)arguments
             scriptController:(LSCScriptController *)scriptController;

@end
