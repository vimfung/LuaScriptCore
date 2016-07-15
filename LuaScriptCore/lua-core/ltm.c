/*
** $Id: ltm.c,v 2.34 2015/03/30 15:42:27 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#include "LuaDefine.h"

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h" 
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"


static const char udatatypename[] = "userdata";

LUAI_DDEF const char *const NameDef(luaT_typenames_)[LUA_TOTALTAGS] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "proto" /* this last case is used for tests only */
};


void NameDef(luaT_init) (NameDef(lua_State) *L) {
  static const char *const luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call"
  };
  int i;
  for (i=0; i<NameDef(TM_N); i++) {
    G(L)->tmname[i] = NameDef(luaS_new)(L, luaT_eventname[i]);
    NameDef(luaC_fix)(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const NameDef(TValue) *NameDef(luaT_gettm) (NameDef(Table) *events, NameDef(TMS) event, NameDef(TString) *ename) {
  const NameDef(TValue) *tm = NameDef(luaH_getstr)(events, ename);
  lua_assert(event <= TM_EQ);
  if (ttisnil(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    return NULL;
  }
  else return tm;
}


const NameDef(TValue) *NameDef(luaT_gettmbyobj) (NameDef(lua_State) *L, const NameDef(TValue) *o, NameDef(TMS) event) {
  NameDef(Table) *mt;
  switch (ttnov(o)) {
    case LUA_TTABLE:
      mt = hvalue(o)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      break;
    default:
      mt = G(L)->mt[ttnov(o)];
  }
  return (mt ? NameDef(luaH_getstr)(mt, G(L)->tmname[event]) : luaO_nilobject);
}


void NameDef(luaT_callTM) (NameDef(lua_State) *L, const NameDef(TValue) *f, const NameDef(TValue) *p1,
                  const NameDef(TValue) *p2, NameDef(TValue) *p3, int hasres) {
  ptrdiff_t result = savestack(L, p3);
  setobj2s(L, L->top++, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, L->top++, p1);  /* 1st argument */
  setobj2s(L, L->top++, p2);  /* 2nd argument */
  if (!hasres)  /* no result? 'p3' is third argument */
    setobj2s(L, L->top++, p3);  /* 3rd argument */
  /* metamethod may yield only when called from Lua code */
  NameDef(luaD_call)(L, L->top - (4 - hasres), hasres, isLua(L->ci));
  if (hasres) {  /* if has result, move it to its place */
    p3 = restorestack(L, result);
    setobjs2s(L, p3, --L->top);
  }
}


int NameDef(luaT_callbinTM) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2,
                    NameDef(StkId) res, NameDef(TMS) event) {
  const NameDef(TValue) *tm = NameDef(luaT_gettmbyobj)(L, p1, event);  /* try first operand */
  if (ttisnil(tm))
    tm = NameDef(luaT_gettmbyobj)(L, p2, event);  /* try second operand */
  if (ttisnil(tm)) return 0;
  NameDef(luaT_callTM)(L, tm, p1, p2, res, 1);
  return 1;
}


void NameDef(luaT_trybinTM) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2,
                    NameDef(StkId) res, NameDef(TMS) event) {
  if (!NameDef(luaT_callbinTM)(L, p1, p2, res, event)) {
    switch (event) {
      case NameDef(TM_CONCAT):
        NameDef(luaG_concaterror)(L, p1, p2);
      /* call never returns, but to avoid warnings: *//* FALLTHROUGH */
      case NameDef(TM_BAND): case NameDef(TM_BOR): case NameDef(TM_BXOR):
      case NameDef(TM_SHL): case NameDef(TM_SHR): case NameDef(TM_BNOT): {
        NameDef(lua_Number) dummy;
        if (tonumber(p1, &dummy) && tonumber(p2, &dummy))
          NameDef(luaG_tointerror)(L, p1, p2);
        else
          NameDef(luaG_opinterror)(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      default:
        NameDef(luaG_opinterror)(L, p1, p2, "perform arithmetic on");
    }
  }
}


int NameDef(luaT_callorderTM) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2,
                      NameDef(TMS) event) {
  if (!NameDef(luaT_callbinTM)(L, p1, p2, L->top, event))
    return -1;  /* no metamethod */
  else
    return !l_isfalse(L->top);
}

