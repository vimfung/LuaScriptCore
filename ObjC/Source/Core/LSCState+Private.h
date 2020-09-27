//
//  LSCState+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/14.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCState.h"

NS_ASSUME_NONNULL_BEGIN

@interface LSCState ()

/**
 Lua层State
 */
@property (nonatomic) lua_State *rawState;


/**
 初始化对象

 @param rawState 原始状态
 @return 对象实例
 */
- (instancetype)_initWithRawState:(lua_State *)rawState;

/**
 放入一个状态到状态映射表
 
 @param state 状态对象
 */
+ (void)_pushStatesMapWithState:(LSCState *)state;

@end

NS_ASSUME_NONNULL_END
