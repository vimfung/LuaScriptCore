/*
** $Id: linit.c,v 1.38 2015/01/05 13:48:33 roberto Exp $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

/*
** If you embed Lua in your program and need to open the standard
** libraries, call luaL_openlibs in your program. If you need a
** different set of libraries, copy this file to your project and edit
** it to suit your needs.
**
** You can also *preload* libraries, so that a later 'require' can
** open the library, which is already linked to the application.
** For that, do the following code:
**
**  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
**  lua_pushcfunction(L, luaopen_modname);
**  lua_setfield(L, -2, modname);
**  lua_pop(L, 1);  // remove _PRELOAD table
*/

#include "LuaDefine.h"

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"


/*
** these libs are loaded by lua.c and are readily available to any Lua
** program
*/
static const NameDef(luaL_Reg) loadedlibs[] = {
  {"_G", NameDef(luaopen_base)},
  {LUA_LOADLIBNAME, NameDef(luaopen_package)},
  {LUA_COLIBNAME, NameDef(luaopen_coroutine)},
  {LUA_TABLIBNAME, NameDef(luaopen_table)},
  {LUA_IOLIBNAME, NameDef(luaopen_io)},
  {LUA_OSLIBNAME, NameDef(luaopen_os)},
  {LUA_STRLIBNAME, NameDef(luaopen_string)},
  {LUA_MATHLIBNAME, NameDef(luaopen_math)},
  {LUA_UTF8LIBNAME, NameDef(luaopen_utf8)},
  {LUA_DBLIBNAME, NameDef(luaopen_debug)},
#if defined(LUA_COMPAT_BITLIB)
  {LUA_BITLIBNAME, NameDef(luaopen_bit32)},
#endif
  {NULL, NULL}
};


LUALIB_API void NameDef(luaL_openlibs) (NameDef(lua_State) *L) {
  const NameDef(luaL_Reg) *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    NameDef(luaL_requiref)(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}

