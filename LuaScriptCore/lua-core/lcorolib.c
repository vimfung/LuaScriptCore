/*
** $Id: lcorolib.c,v 1.9 2014/11/02 19:19:04 roberto Exp $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#include "LuaDefine.h"

#include "lprefix.h"


#include <stdlib.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static NameDef(lua_State) *getco (NameDef(lua_State) *L) {
  NameDef(lua_State) *co = NameDef(lua_tothread)(L, 1);
  luaL_argcheck(L, co, 1, "thread expected");
  return co;
}


static int auxresume (NameDef(lua_State) *L, NameDef(lua_State) *co, int narg) {
  int status;
  if (!NameDef(lua_checkstack)(co, narg)) {
    lua_pushliteral(L, "too many arguments to resume");
    return -1;  /* error flag */
  }
  if (NameDef(lua_status)(co) == LUA_OK && NameDef(lua_gettop)(co) == 0) {
    lua_pushliteral(L, "cannot resume dead coroutine");
    return -1;  /* error flag */
  }
  NameDef(lua_xmove)(L, co, narg);
  status = NameDef(lua_resume)(co, L, narg);
  if (status == LUA_OK || status == LUA_YIELD) {
    int nres = NameDef(lua_gettop)(co);
    if (!NameDef(lua_checkstack)(L, nres + 1)) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      return -1;  /* error flag */
    }
    NameDef(lua_xmove)(co, L, nres);  /* move yielded values */
    return nres;
  }
  else {
    NameDef(lua_xmove)(co, L, 1);  /* move error message */
    return -1;  /* error flag */
  }
}


static int luaB_coresume (NameDef(lua_State) *L) {
  NameDef(lua_State) *co = getco(L);
  int r;
  r = auxresume(L, co, NameDef(lua_gettop)(L) - 1);
  if (r < 0) {
    NameDef(lua_pushboolean)(L, 0);
    lua_insert(L, -2);
    return 2;  /* return false + error message */
  }
  else {
    NameDef(lua_pushboolean)(L, 1);
    lua_insert(L, -(r + 1));
    return r + 1;  /* return true + 'resume' returns */
  }
}


static int luaB_auxwrap (NameDef(lua_State) *L) {
  NameDef(lua_State) *co = NameDef(lua_tothread)(L, lua_upvalueindex(1));
  int r = auxresume(L, co, NameDef(lua_gettop)(L));
  if (r < 0) {
    if (NameDef(lua_isstring)(L, -1)) {  /* error object is a string? */
      NameDef(luaL_where)(L, 1);  /* add extra info */
      lua_insert(L, -2);
      NameDef(lua_concat)(L, 2);
    }
    return NameDef(lua_error)(L);  /* propagate error */
  }
  return r;
}


static int luaB_cocreate (NameDef(lua_State) *L) {
  NameDef(lua_State) *NL;
  NameDef(luaL_checktype)(L, 1, LUA_TFUNCTION);
  NL = NameDef(lua_newthread)(L);
  NameDef(lua_pushvalue)(L, 1);  /* move function to top */
  NameDef(lua_xmove)(L, NL, 1);  /* move function from L to NL */
  return 1;
}


static int luaB_cowrap (NameDef(lua_State) *L) {
  luaB_cocreate(L);
  NameDef(lua_pushcclosure)(L, luaB_auxwrap, 1);
  return 1;
}


static int luaB_yield (NameDef(lua_State) *L) {
  return lua_yield(L, NameDef(lua_gettop)(L));
}


static int luaB_costatus (NameDef(lua_State) *L) {
  NameDef(lua_State) *co = getco(L);
  if (L == co) lua_pushliteral(L, "running");
  else {
    switch (NameDef(lua_status)(co)) {
      case LUA_YIELD:
        lua_pushliteral(L, "suspended");
        break;
      case LUA_OK: {
        NameDef(lua_Debug) ar;
        if (NameDef(lua_getstack)(co, 0, &ar) > 0)  /* does it have frames? */
          lua_pushliteral(L, "normal");  /* it is running */
        else if (NameDef(lua_gettop)(co) == 0)
            lua_pushliteral(L, "dead");
        else
          lua_pushliteral(L, "suspended");  /* initial state */
        break;
      }
      default:  /* some error occurred */
        lua_pushliteral(L, "dead");
        break;
    }
  }
  return 1;
}


static int luaB_yieldable (NameDef(lua_State) *L) {
  NameDef(lua_pushboolean)(L, NameDef(lua_isyieldable)(L));
  return 1;
}


static int luaB_corunning (NameDef(lua_State) *L) {
  int ismain = NameDef(lua_pushthread)(L);
  NameDef(lua_pushboolean)(L, ismain);
  return 2;
}


static const NameDef(luaL_Reg) co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {NULL, NULL}
};



LUAMOD_API int NameDef(luaopen_coroutine) (NameDef(lua_State) *L) {
  luaL_newlib(L, co_funcs);
  return 1;
}

