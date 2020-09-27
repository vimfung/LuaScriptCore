//
//  LSCScriptControlWatcher.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/26.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCStateWatcher.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCScriptControlWatcher;

/**
 脚本退出事件处理器

 @param watcher 监视器
 @param context 上下文
 */
typedef void (^LSCScriptExitEventHandler) (LSCScriptControlWatcher *watcher, LSCContext *context);


/**
 脚本超时事件处理器

 @param watcher 监视器
 @param context 上下文
 */
typedef void (^LSCScriptTimeoutEventHandler) (LSCScriptControlWatcher *watcher, LSCContext *context);

/**
 脚本控制监视器，用于控制脚本的执行
 */
@interface LSCScriptControlWatcher : LSCStateWatcher

/**
 脚本超时时间，默认为0，表示不限制时长
 */
@property (nonatomic) NSTimeInterval scriptTimeout;

/**
 退出脚本时触发

 @param handler 事件处理器
 */
- (void)onExit:(LSCScriptExitEventHandler)handler;

/**
 脚本超时时触发

 @param handler 事件处理器
 */
- (void)onTimeout:(LSCScriptTimeoutEventHandler)handler;

/**
 退出脚本
 */
- (void)exitScript;

/**
 重置监视器，每次执行脚本前都应该调用一次重置，避免上次状态影响到当前的执行判定
 */
- (void)reset;

@end

NS_ASSUME_NONNULL_END
