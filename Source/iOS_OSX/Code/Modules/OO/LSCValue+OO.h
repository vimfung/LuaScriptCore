//
//  LSCValue+OO.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "lua.h"


/**
 面向对象模块扩展
 */
@interface LSCValue (OO)

/**
 *  获取栈中的某个值
 *
 *  @param state 状态机
 *  @param index 栈索引
 *
 *  @return 值对象
 */
+ (LSCValue *)objectValueWithState:(lua_State *)state atIndex:(NSInteger)index;

@end
