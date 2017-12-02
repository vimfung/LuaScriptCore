//
//  lext.c
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/8/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#include "lext.h"
#include "lstate.h"

#define ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

LUA_API int lua_absindex (lua_State *L, int idx) {
    return (idx > 0 || ispseudo(idx))
    ? idx
    : cast_int(L->top - L->ci->func) + idx;
}
