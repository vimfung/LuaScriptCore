//
//  LSCContext.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCException.h"
#import "LSCValueType.h"
#import "LSCModule.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCStateWatcher;

/**
 上下文对象
 */
@interface LSCContext : NSObject

/**
 状态监视器
 */
@property (nonatomic, strong, nullable) LSCStateWatcher *stateWatcher;

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
 *  @return 返回值
 */
- (id<LSCValueType>)evalScriptFromString:(NSString *)string;


/**
 *  解析脚本
 *
 *  @param path 脚本路径
 *
 *  @return 返回值
 */
- (id<LSCValueType>)evalScriptFromFile:(NSString *)path;

/**
 设置全局变量

 @param value 变量值
 @param name 变量名称
 */
- (void)setGlobal:(nullable id<LSCValueType>)value forName:(NSString *)name;

/**
 获取全局变量

 @param name 变量名称
 */
- (id<LSCValueType>)getGlobalForName:(NSString *)name;

/**
 回收内存
 */
- (void)gc;

#pragma mark Exception Methods

/**
 *  发生异常时触发
 *
 *  @param handler 事件处理器
 */
- (void)onException:(LSCExceptionHandler)handler;

/**
 抛出异常

 @param message 异常消息
 */
- (void)raiseExceptionWithMessage:(NSString *)message;

#pragma mark Module Methods

/**
 使用功能模块
 
 @param module 模块
 */
- (void)useModule:(Class<LSCModule>)module;

/**
 获取模块自定义数据

 @param key 数据标识
 @param module 模块
 @return 数据
 */
- (id)getUserdataWithKey:(NSString *)key
                  module:(Class<LSCModule>)module;

/**
 设置模块自定义数据

 @param userdata 自定义数据
 @param key 数据标识
 @param module 模块
 */
- (void)setUserdata:(id)userdata
             forKey:(NSString *)key
             module:(Class<LSCModule>)module;

@end

NS_ASSUME_NONNULL_END
