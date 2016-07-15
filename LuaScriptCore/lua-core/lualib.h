/*
** $Id: lualib.h,v 1.44 2014/02/06 17:32:33 roberto Exp $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "LuaDefine.h"
#include "lua.h"



LUAMOD_API int (NameDef(luaopen_base)) (NameDef(lua_State) *L);

#define LUA_COLIBNAME	"coroutine"
LUAMOD_API int (NameDef(luaopen_coroutine)) (NameDef(lua_State) *L);

#define LUA_TABLIBNAME	"table"
LUAMOD_API int (NameDef(luaopen_table)) (NameDef(lua_State) *L);

#define LUA_IOLIBNAME	"io"
LUAMOD_API int (NameDef(luaopen_io)) (NameDef(lua_State) *L);

#define LUA_OSLIBNAME	"os"
LUAMOD_API int (NameDef(luaopen_os)) (NameDef(lua_State) *L);

#define LUA_STRLIBNAME	"string"
LUAMOD_API int (NameDef(luaopen_string)) (NameDef(lua_State) *L);

#define LUA_UTF8LIBNAME	"utf8"
LUAMOD_API int (NameDef(luaopen_utf8)) (NameDef(lua_State) *L);

#define LUA_BITLIBNAME	"bit32"
LUAMOD_API int (NameDef(luaopen_bit32)) (NameDef(lua_State) *L);

#define LUA_MATHLIBNAME	"math"
LUAMOD_API int (NameDef(luaopen_math)) (NameDef(lua_State) *L);

#define LUA_DBLIBNAME	"debug"
LUAMOD_API int (NameDef(luaopen_debug)) (NameDef(lua_State) *L);

#define LUA_LOADLIBNAME	"package"
LUAMOD_API int (NameDef(luaopen_package)) (NameDef(lua_State) *L);


/* open all previous libraries */
LUALIB_API void (NameDef(luaL_openlibs)) (NameDef(lua_State) *L);



#if !defined(lua_assert)
#define lua_assert(x)	((void)0)
#endif


#endif
