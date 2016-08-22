/*
** $Id: ltm.h,v 2.22 2016/02/26 19:20:15 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#include "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
typedef enum {
  NameDef(TM_INDEX),
  NameDef(TM_NEWINDEX),
  NameDef(TM_GC),
  NameDef(TM_MODE),
  NameDef(TM_LEN),
  NameDef(TM_EQ),  /* last tag method with fast access */
  NameDef(TM_ADD),
  NameDef(TM_SUB),
  NameDef(TM_MUL),
  NameDef(TM_MOD),
  NameDef(TM_POW),
  NameDef(TM_DIV),
  NameDef(TM_IDIV),
  NameDef(TM_BAND),
  NameDef(TM_BOR),
  NameDef(TM_BXOR),
  NameDef(TM_SHL),
  NameDef(TM_SHR),
  NameDef(TM_UNM),
  NameDef(TM_BNOT),
  NameDef(TM_LT),
  NameDef(TM_LE),
  NameDef(TM_CONCAT),
  NameDef(TM_CALL),
  NameDef(TM_N)		/* number of elements in the enum */
} NameDef(TMS);



#define gfasttm(g,et,e) ((et) == NULL ? NULL : \
  ((et)->flags & (1u<<(e))) ? NULL : NameDef(luaT_gettm)(et, e, (g)->tmname[e]))

#define fasttm(l,et,e)	gfasttm(G(l), et, e)

#define ttypename(x)	NameDef(luaT_typenames_)[(x) + 1]

LUAI_DDEC const char *const NameDef(luaT_typenames_)[LUA_TOTALTAGS];


LUAI_FUNC const char *NameDef(luaT_objtypename) (NameDef(lua_State) *L, const NameDef(TValue) *o);

LUAI_FUNC const NameDef(TValue) *NameDef(luaT_gettm) (NameDef(Table) *events, NameDef(TMS) event, NameDef(TString) *ename);
LUAI_FUNC const NameDef(TValue) *NameDef(luaT_gettmbyobj) (NameDef(lua_State) *L, const NameDef(TValue) *o,
                                                       NameDef(TMS) event);
LUAI_FUNC void NameDef(luaT_init) (NameDef(lua_State) *L);

LUAI_FUNC void NameDef(luaT_callTM) (NameDef(lua_State) *L, const NameDef(TValue) *f, const NameDef(TValue) *p1,
                            const NameDef(TValue) *p2, NameDef(TValue) *p3, int hasres);
LUAI_FUNC int NameDef(luaT_callbinTM) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2,
                              NameDef(StkId) res, NameDef(TMS) event);
LUAI_FUNC void NameDef(luaT_trybinTM) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2,
                              NameDef(StkId) res, NameDef(TMS) event);
LUAI_FUNC int NameDef(luaT_callorderTM) (NameDef(lua_State) *L, const NameDef(TValue) *p1,
                                const NameDef(TValue) *p2, NameDef(TMS) event);



#endif
