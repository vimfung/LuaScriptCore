//
//  LuaState.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "lua.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCStateManager;
@class LSCStateWatcher;

@interface LSCState : NSObject

/**
 Lua层State
 */
@property (nonatomic, readonly) lua_State *rawState;

/**
 设置观察器,用于监控lua运行状态
 */
@property (nonatomic, strong, nullable) LSCStateWatcher *watcher;

/**
 获取状态对象
 
 @param rawState 原始状态对象
 @return 状态对象实例
 */
+ (LSCState *)stateWithRawState:(lua_State *)rawState;

//- (void)watch;

@end

NS_ASSUME_NONNULL_END
