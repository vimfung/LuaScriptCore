//
//  LUAValue_Private.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "lua.h"

@interface LSCValue ()

/**
 *  获取栈中的某个值
 *
 *  @param state 状态机
 *  @param index 栈索引
 *
 *  @return 值对象
 */
+ (LSCValue *)valueWithState:(lua_State *)state atIndex:(NSInteger)index;

/**
 *  入栈数据
 *
 *  @param state Lua解析器
 */
- (void)pushWithState:(lua_State *)state;

@end
