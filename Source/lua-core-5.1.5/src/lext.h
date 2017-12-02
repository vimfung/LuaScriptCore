//
//  lext.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/8/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#ifndef lext_h
#define lext_h

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

LUA_API int (lua_absindex) (lua_State *L, int idx);

#ifdef __cplusplus
}
#endif

#endif /* lext_h */
