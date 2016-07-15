/*
** $Id: ldebug.h,v 2.14 2015/05/22 17:45:56 roberto Exp $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h

#include "LuaDefine.h"
#include "lstate.h"


#define pcRel(pc, p)	(cast(int, (pc) - (p)->code) - 1)

#define getfuncline(f,pc)	(((f)->lineinfo) ? (f)->lineinfo[pc] : -1)

#define resethookcount(L)	(L->hookcount = L->basehookcount)


LUAI_FUNC l_noret NameDef(luaG_typeerror) (NameDef(lua_State) *L, const NameDef(TValue) *o,
                                                const char *opname);
LUAI_FUNC l_noret NameDef(luaG_concaterror) (NameDef(lua_State) *L, const NameDef(TValue) *p1,
                                                  const NameDef(TValue) *p2);
LUAI_FUNC l_noret NameDef(luaG_opinterror) (NameDef(lua_State) *L, const NameDef(TValue) *p1,
                                                 const NameDef(TValue) *p2,
                                                 const char *msg);
LUAI_FUNC l_noret NameDef(luaG_tointerror) (NameDef(lua_State) *L, const NameDef(TValue) *p1,
                                                 const NameDef(TValue) *p2);
LUAI_FUNC l_noret NameDef(luaG_ordererror) (NameDef(lua_State) *L, const NameDef(TValue) *p1,
                                                 const NameDef(TValue) *p2);
LUAI_FUNC l_noret NameDef(luaG_runerror) (NameDef(lua_State) *L, const char *fmt, ...);
LUAI_FUNC const char *NameDef(luaG_addinfo) (NameDef(lua_State) *L, const char *msg,
                                                  NameDef(TString) *src, int line);
LUAI_FUNC l_noret NameDef(luaG_errormsg) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaG_traceexec) (NameDef(lua_State) *L);


#endif
