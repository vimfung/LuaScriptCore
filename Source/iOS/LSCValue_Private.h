//
//  LUAValue_Private.h
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "lua/lua.h"

@interface LSCValue ()

/**
 *  入栈数据
 *
 *  @param state Lua解析器
 */
- (void)pushWithState:(lua_State *)state;

@end
