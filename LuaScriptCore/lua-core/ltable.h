/*
** $Id: ltable.h,v 2.20 2014/09/04 18:15:29 roberto Exp $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ifndef ltable_h
#define ltable_h

#include "LuaDefine.h"
#include "lobject.h"


#define gnode(t,i)	(&(t)->node[i])
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->i_key.nk.next)


/* 'const' to avoid wrong writings that can mess up field 'next' */ 
#define gkey(n)		cast(const NameDef(TValue)*, (&(n)->i_key.tvk))

#define wgkey(n)		(&(n)->i_key.nk)

#define invalidateTMcache(t)	((t)->flags = 0)


/* returns the key, given the value of a table entry */
#define keyfromval(v) \
  (gkey(cast(NameDef(Node) *, cast(char *, (v)) - offsetof(NameDef(Node), i_val))))


LUAI_FUNC const NameDef(TValue) *NameDef(luaH_getint) (NameDef(Table) *t, NameDef(lua_Integer) key);
LUAI_FUNC void NameDef(luaH_setint) (NameDef(lua_State) *L, NameDef(Table) *t, NameDef(lua_Integer) key,
                                                    NameDef(TValue) *value);
LUAI_FUNC const NameDef(TValue) *NameDef(luaH_getstr) (NameDef(Table) *t, NameDef(TString) *key);
LUAI_FUNC const NameDef(TValue) *NameDef(luaH_get) (NameDef(Table) *t, const NameDef(TValue) *key);
LUAI_FUNC NameDef(TValue) *NameDef(luaH_newkey) (NameDef(lua_State) *L, NameDef(Table) *t, const NameDef(TValue) *key);
LUAI_FUNC NameDef(TValue) *NameDef(luaH_set) (NameDef(lua_State) *L, NameDef(Table) *t, const NameDef(TValue) *key);
LUAI_FUNC NameDef(Table) *NameDef(luaH_new) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaH_resize) (NameDef(lua_State) *L, NameDef(Table) *t, unsigned int nasize,
                                                    unsigned int nhsize);
LUAI_FUNC void NameDef(luaH_resizearray) (NameDef(lua_State) *L, NameDef(Table) *t, unsigned int nasize);
LUAI_FUNC void NameDef(luaH_free) (NameDef(lua_State) *L, NameDef(Table) *t);
LUAI_FUNC int NameDef(luaH_next) (NameDef(lua_State) *L, NameDef(Table) *t, NameDef(StkId) key);
LUAI_FUNC int NameDef(luaH_getn) (NameDef(Table) *t);


#if defined(LUA_DEBUG)
LUAI_FUNC NameDef(Node) *NameDef(luaH_mainposition) (const NameDef(Table) *t, const NameDef(TValue) *key);
LUAI_FUNC int NameDef(luaH_isdummy) (NameDef(Node) *n);
#endif


#endif
