/*
** $Id: ldo.h,v 2.29 2015/12/21 13:02:14 roberto Exp $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#include "lobject.h"
#include "lstate.h"
#include "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/
#define luaD_checkstackaux(L,n,pre,pos)  \
	if (L->stack_last - L->top <= (n)) \
	  { pre; NameDef(luaD_growstack)(L, n); pos; } else { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(void)0,(void)0)



#define savestack(L,p)		((char *)(p) - (char *)L->stack)
#define restorestack(L,n)	((NameDef(TValue) *)((char *)L->stack + (n)))


/* type of protected functions, to be ran by 'runprotected' */
typedef void (*NameDef(Pfunc)) (NameDef(lua_State) *L, void *ud);

LUAI_FUNC int NameDef(luaD_protectedparser) (NameDef(lua_State) *L, NameDef(ZIO) *z, const char *name,
                                                  const char *mode);
LUAI_FUNC void NameDef(luaD_hook) (NameDef(lua_State) *L, int event, int line);
LUAI_FUNC int NameDef(luaD_precall) (NameDef(lua_State) *L, NameDef(StkId) func, int nresults);
LUAI_FUNC void NameDef(luaD_call) (NameDef(lua_State) *L, NameDef(StkId) func, int nResults);
LUAI_FUNC void NameDef(luaD_callnoyield) (NameDef(lua_State) *L, NameDef(StkId) func, int nResults);
LUAI_FUNC int NameDef(luaD_pcall) (NameDef(lua_State) *L, NameDef(Pfunc) func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC int NameDef(luaD_poscall) (NameDef(lua_State) *L, NameDef(CallInfo) *ci, NameDef(StkId) firstResult,
                                          int nres);
LUAI_FUNC void NameDef(luaD_reallocstack )(NameDef(lua_State) *L, int newsize);
LUAI_FUNC void NameDef(luaD_growstack) (NameDef(lua_State) *L, int n);
LUAI_FUNC void NameDef(luaD_shrinkstack) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaD_inctop) (NameDef(lua_State) *L);

LUAI_FUNC l_noret NameDef(luaD_throw) (NameDef(lua_State) *L, int errcode);
LUAI_FUNC int NameDef(luaD_rawrunprotected) (NameDef(lua_State) *L, NameDef(Pfunc) f, void *ud);

#endif

