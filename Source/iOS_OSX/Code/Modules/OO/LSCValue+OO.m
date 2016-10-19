//
//  LSCValue+OO.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue+OO.h"
#import "LSCValue_Private.h"

@implementation LSCValue (OO)

+ (LSCValue *)objectValueWithState:(lua_State *)state atIndex:(NSInteger)index
{
    LSCValue *value = nil;
    
    //先判断是否为类实例对象
    if (lua_type(state, (int)index) == LUA_TTABLE)
    {
        lua_getfield(state, index, "_nativeObject");
        if (lua_type(state, -1) == LUA_TLIGHTUSERDATA)
        {
            //为类实例对象
            id obj = (__bridge id)lua_topointer(state, -1);
            value = [LSCValue objectValue:obj];
        }
        lua_pop(state, 1);
    }
    
    if (!value)
    {
        value = [LSCValue valueWithState:state atIndex:index];
    }
    
    return value;
}

@end
