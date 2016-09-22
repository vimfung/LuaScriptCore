//
//  LSCClassInstance_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/20.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCClassInstance.h"
#import "lua.h"

@interface LSCClassInstance ()

/**
 *  状态机
 */
@property (nonatomic) lua_State *state;

/**
 *  所处的索引位置
 */
@property (nonatomic) int index;

/**
 *  所属本地类型
 */
@property (nonatomic) Class ownerClass;

/**
 *  本地对象
 */
@property (nonatomic, strong) LSCObjectClass *nativeObject;

/**
 *  初始化类实例对象
 *
 *  @param state 状态机
 *  @param index 实例所在索引
 *
 *  @return 实例对象
 */
- (instancetype)initWithState:(lua_State *)state atIndex:(int)index;

@end
