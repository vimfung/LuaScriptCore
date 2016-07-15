/*
** $Id: lfunc.h,v 2.15 2015/01/13 15:49:11 roberto Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h

#include "LuaDefine.h"

#include "lobject.h"


#define sizeCclosure(n)	(cast(int, sizeof(NameDef(CClosure))) + \
                         cast(int, sizeof(NameDef(TValue))*((n)-1)))

#define sizeLclosure(n)	(cast(int, sizeof(NameDef(LClosure))) + \
                         cast(int, sizeof(NameDef(TValue) *)*((n)-1)))


/* test whether thread is in 'twups' list */
#define isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#define MAXUPVAL	255


/*
** Upvalues for Lua closures
*/
struct NameDef(UpVal) {
  NameDef(TValue) *v;  /* points to stack or to its own value */
  NameDef(lu_mem) refcount;  /* reference counter */
  union {
    struct {  /* (when open) */
      NameDef(UpVal) *next;  /* linked list */
      int touched;  /* mark to avoid cycles with dead threads */
    } open;
    NameDef(TValue) value;  /* the value (when closed) */
  } u;
};

#define upisopen(up)	((up)->v != &(up)->u.value)


LUAI_FUNC NameDef(Proto) *NameDef(luaF_newproto) (NameDef(lua_State) *L);
LUAI_FUNC NameDef(CClosure) *NameDef(luaF_newCclosure) (NameDef(lua_State) *L, int nelems);
LUAI_FUNC NameDef(LClosure) *NameDef(luaF_newLclosure) (NameDef(lua_State) *L, int nelems);
LUAI_FUNC void NameDef(luaF_initupvals) (NameDef(lua_State) *L, NameDef(LClosure) *cl);
LUAI_FUNC NameDef(UpVal) *NameDef(luaF_findupval) (NameDef(lua_State) *L, NameDef(StkId) level);
LUAI_FUNC void NameDef(luaF_close) (NameDef(lua_State) *L, NameDef(StkId) level);
LUAI_FUNC void NameDef(luaF_freeproto) (NameDef(lua_State) *L, NameDef(Proto) *f);
LUAI_FUNC const char *NameDef(luaF_getlocalname) (const NameDef(Proto) *func, int local_number,
                                         int pc);


#endif
