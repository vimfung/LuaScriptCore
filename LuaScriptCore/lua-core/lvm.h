/*
** $Id: lvm.h,v 2.35 2015/02/20 14:27:53 roberto Exp $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ifndef lvm_h
#define lvm_h

#include "LuaDefine.h"

#include "ldo.h"
#include "lobject.h"
#include "ltm.h"


#if !defined(LUA_NOCVTN2S)
#define cvt2str(o)	ttisnumber(o)
#else
#define cvt2str(o)	0	/* no conversion from numbers to strings */
#endif


#if !defined(LUA_NOCVTS2N)
#define cvt2num(o)	ttisstring(o)
#else
#define cvt2num(o)	0	/* no conversion from strings to numbers */
#endif


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#if !defined(LUA_FLOORN2I)
#define LUA_FLOORN2I		0
#endif


#define tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : NameDef(luaV_tonumber_)(o,n))

#define tointeger(o,i) \
    (ttisinteger(o) ? (*(i) = ivalue(o), 1) : NameDef(luaV_tointeger)(o,i,LUA_FLOORN2I))

#define intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#define luaV_rawequalobj(t1,t2)		NameDef(luaV_equalobj)(NULL,t1,t2)


LUAI_FUNC int NameDef(luaV_equalobj) (NameDef(lua_State) *L, const NameDef(TValue) *t1, const NameDef(TValue) *t2);
LUAI_FUNC int NameDef(luaV_lessthan) (NameDef(lua_State) *L, const NameDef(TValue) *l, const NameDef(TValue) *r);
LUAI_FUNC int NameDef(luaV_lessequal) (NameDef(lua_State) *L, const NameDef(TValue) *l, const NameDef(TValue) *r);
LUAI_FUNC int NameDef(luaV_tonumber_) (const NameDef(TValue) *obj, NameDef(lua_Number) *n);
LUAI_FUNC int NameDef(luaV_tointeger) (const NameDef(TValue) *obj, NameDef(lua_Integer) *p, int mode);
LUAI_FUNC void NameDef(luaV_gettable) (NameDef(lua_State) *L, const NameDef(TValue) *t, NameDef(TValue) *key,
                                            NameDef(StkId) val);
LUAI_FUNC void NameDef(luaV_settable) (NameDef(lua_State) *L, const NameDef(TValue) *t, NameDef(TValue) *key,
                                            NameDef(StkId) val);
LUAI_FUNC void NameDef(luaV_finishOp) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaV_execute) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaV_concat) (NameDef(lua_State) *L, int total);
LUAI_FUNC NameDef(lua_Integer) NameDef(luaV_div) (NameDef(lua_State) *L, NameDef(lua_Integer) x, NameDef(lua_Integer) y);
LUAI_FUNC NameDef(lua_Integer) NameDef(luaV_mod) (NameDef(lua_State) *L, NameDef(lua_Integer) x, NameDef(lua_Integer) y);
LUAI_FUNC NameDef(lua_Integer) NameDef(luaV_shiftl) (NameDef(lua_Integer) x, NameDef(lua_Integer) y);
LUAI_FUNC void NameDef(luaV_objlen) (NameDef(lua_State) *L, NameDef(StkId) ra, const NameDef(TValue) *rb);

#endif
