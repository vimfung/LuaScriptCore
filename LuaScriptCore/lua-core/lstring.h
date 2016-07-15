/*
** $Id: lstring.h,v 1.59 2015/03/25 13:42:19 roberto Exp $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h

#include "LuaDefine.h"

#include "lgc.h"
#include "lobject.h"
#include "lstate.h"


#define sizelstring(l)  (sizeof(union NameDef(UTString)) + ((l) + 1) * sizeof(char))

#define sizeludata(l)	(sizeof(union NameDef(UUdata)) + (l))
#define sizeudata(u)	sizeludata((u)->len)

#define luaS_newliteral(L, s)	(NameDef(luaS_newlstr)(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))


/*
** test whether a string is a reserved word
*/
#define isreserved(s)	((s)->tt == LUA_TSHRSTR && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#define eqshrstr(a,b)	check_exp((a)->tt == LUA_TSHRSTR, (a) == (b))


LUAI_FUNC unsigned int  NameDef(luaS_hash) (const char *str, size_t l, unsigned int seed);
LUAI_FUNC int  NameDef(luaS_eqlngstr) (NameDef(TString) *a, NameDef(TString) *b);
LUAI_FUNC void  NameDef(luaS_resize) (NameDef(lua_State) *L, int newsize);
LUAI_FUNC void  NameDef(luaS_clearcache) (NameDef(global_State) *g);
LUAI_FUNC void  NameDef(luaS_init) (NameDef(lua_State) *L);
LUAI_FUNC void  NameDef(luaS_remove) (NameDef(lua_State) *L, NameDef(TString) *ts);
LUAI_FUNC NameDef(Udata) * NameDef(luaS_newudata) (NameDef(lua_State) *L, size_t s);
LUAI_FUNC NameDef(TString) * NameDef(luaS_newlstr) (NameDef(lua_State) *L, const char *str, size_t l);
LUAI_FUNC NameDef(TString) * NameDef(luaS_new) (NameDef(lua_State) *L, const char *str);


#endif
