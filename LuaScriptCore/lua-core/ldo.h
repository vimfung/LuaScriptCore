/*
** $Id: ldo.h,v 2.22 2015/05/22 17:48:19 roberto Exp $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h

#include "LuaDefine.h"

#include "lobject.h"
#include "lstate.h"
#include "lzio.h"


#define luaD_checkstack(L,n)	if (L->stack_last - L->top <= (n)) \
				    NameDef(luaD_growstack)(L, n); else condmovestack(L);


#define incr_top(L) {L->top++; luaD_checkstack(L,0);}

#define savestack(L,p)		((char *)(p) - (char *)L->stack)
#define restorestack(L,n)	((NameDef(TValue) *)((char *)L->stack + (n)))


/* type of protected functions, to be ran by 'runprotected' */
typedef void (*NameDef(Pfunc)) (NameDef(lua_State) *L, void *ud);

LUAI_FUNC int NameDef(luaD_protectedparser) (NameDef(lua_State) *L, NameDef(ZIO) *z, const char *name,
                                                  const char *mode);
LUAI_FUNC void NameDef(luaD_hook) (NameDef(lua_State) *L, int event, int line);
LUAI_FUNC int NameDef(luaD_precall) (NameDef(lua_State) *L, NameDef(StkId) func, int nresults);
LUAI_FUNC void NameDef(luaD_call) (NameDef(lua_State) *L, NameDef(StkId) func, int nResults,
                                        int allowyield);
LUAI_FUNC int NameDef(luaD_pcall) (NameDef(lua_State) *L, NameDef(Pfunc) func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC int NameDef(luaD_poscall) (NameDef(lua_State) *L, NameDef(StkId) firstResult, int nres);
LUAI_FUNC void NameDef(luaD_reallocstack) (NameDef(lua_State) *L, int newsize);
LUAI_FUNC void NameDef(luaD_growstack) (NameDef(lua_State) *L, int n);
LUAI_FUNC void NameDef(luaD_shrinkstack) (NameDef(lua_State) *L);

LUAI_FUNC l_noret NameDef(luaD_throw) (NameDef(lua_State) *L, int errcode);
LUAI_FUNC int NameDef(luaD_rawrunprotected) (NameDef(lua_State) *L, NameDef(Pfunc) f, void *ud);

#endif

