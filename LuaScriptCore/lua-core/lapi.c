/*
** $Id: lapi.c,v 2.249 2015/04/06 12:23:48 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#include "lprefix.h"


#include <stdarg.h>
#include <string.h>

#include "LuaDefine.h"

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"



const char NameDef(lua_ident)[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";


/* value at a non-valid index */
#define NONVALIDVALUE		cast(NameDef(TValue) *, luaO_nilobject)

/* corresponding test */
#define isvalid(o)	((o) != luaO_nilobject)

/* test for pseudo index */
#define ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#define isupvalue(i)		((i) < LUA_REGISTRYINDEX)

/* test for valid but not pseudo index */
#define isstackindex(i, o)	(isvalid(o) && !ispseudo(i))

#define api_checkvalidindex(l,o)  api_check(l, isvalid(o), "invalid index")

#define api_checkstackindex(l, i, o)  \
	api_check(l, isstackindex(i, o), "index not in the stack")


static NameDef(TValue) *index2addr (NameDef(lua_State) *L, int idx) {
  NameDef(CallInfo) *ci = L->ci;
  if (idx > 0) {
    NameDef(TValue) *o = ci->func + idx;
    api_check(L, idx <= ci->top - (ci->func + 1), "unacceptable index");
    if (o >= L->top) return NONVALIDVALUE;
    else return o;
  }
  else if (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->top - (ci->func + 1), "invalid index");
    return L->top + idx;
  }
  else if (idx == LUA_REGISTRYINDEX)
    return &G(L)->l_registry;
  else {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    if (ttislcf(ci->func))  /* light C function? */
      return NONVALIDVALUE;  /* it has no upvalues */
    else {
      NameDef(CClosure) *func = clCvalue(ci->func);
      return (idx <= func->nupvalues) ? &func->upvalue[idx-1] : NONVALIDVALUE;
    }
  }
}


/*
** to be called by 'lua_checkstack' in protected mode, to grow stack
** capturing memory errors
*/
static void growstack (NameDef(lua_State) *L, void *ud) {
  int size = *(int *)ud;
  NameDef(luaD_growstack)(L, size);
}


LUA_API int NameDef(lua_checkstack) (NameDef(lua_State) *L, int n) {
  int res;
  NameDef(CallInfo) *ci = L->ci;
  lua_lock(L);
  api_check(L, n >= 0, "negative 'n'");
  if (L->stack_last - L->top > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  else {  /* no; need to grow stack */
    int inuse = cast_int(L->top - L->stack) + EXTRA_STACK;
    if (inuse > LUAI_MAXSTACK - n)  /* can grow without overflow? */
      res = 0;  /* no */
    else  /* try to grow stack */
      res = (NameDef(luaD_rawrunprotected)(L, &growstack, &n) == LUA_OK);
  }
  if (res && ci->top < L->top + n)
    ci->top = L->top + n;  /* adjust frame top */
  lua_unlock(L);
  return res;
}


LUA_API void NameDef(lua_xmove) (NameDef(lua_State) *from, NameDef(lua_State) *to, int n) {
  int i;
  if (from == to) return;
  lua_lock(to);
  api_checknelems(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->top - to->top >= n, "not enough elements to move");
  from->top -= n;
  for (i = 0; i < n; i++) {
    setobj2s(to, to->top, from->top + i);
    api_incr_top(to);
  }
  lua_unlock(to);
}


LUA_API NameDef(lua_CFunction) NameDef(lua_atpanic) (NameDef(lua_State) *L, NameDef(lua_CFunction) panicf) {
  NameDef(lua_CFunction) old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  return old;
}


LUA_API const NameDef(lua_Number) *NameDef(lua_version) (NameDef(lua_State) *L) {
  static const NameDef(lua_Number) version = LUA_VERSION_NUM;
  if (L == NULL) return &version;
  else return G(L)->version;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API int NameDef(lua_absindex) (NameDef(lua_State) *L, int idx) {
  return (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->top - L->ci->func) + idx;
}


LUA_API int NameDef(lua_gettop) (NameDef(lua_State) *L) {
  return cast_int(L->top - (L->ci->func + 1));
}


LUA_API void NameDef(lua_settop) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) func = L->ci->func;
  lua_lock(L);
  if (idx >= 0) {
    api_check(L, idx <= L->stack_last - (func + 1), "new top too large");
    while (L->top < (func + 1) + idx)
      setnilvalue(L->top++);
    L->top = (func + 1) + idx;
  }
  else {
    api_check(L, -(idx+1) <= (L->top - (func + 1)), "invalid new top");
    L->top += idx+1;  /* 'subtract' index (index is negative) */
  }
  lua_unlock(L);
}


/*
** Reverse the stack segment from 'from' to 'to'
** (auxiliary to 'lua_rotate')
*/
static void reverse (NameDef(lua_State) *L, NameDef(StkId) from, NameDef(StkId) to) {
  for (; from < to; from++, to--) {
    NameDef(TValue) temp;
    setobj(L, &temp, from);
    setobjs2s(L, from, to);
    setobj2s(L, to, &temp);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API void NameDef(lua_rotate) (NameDef(lua_State) *L, int idx, int n) {
  NameDef(StkId) p, t, m;
  lua_lock(L);
  t = L->top - 1;  /* end of stack segment being rotated */
  p = index2addr(L, idx);  /* start of segment */
  api_checkstackindex(L, idx, p);
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API void NameDef(lua_copy) (NameDef(lua_State) *L, int fromidx, int toidx) {
  NameDef(TValue) *fr, *to;
  lua_lock(L);
  fr = index2addr(L, fromidx);
  to = index2addr(L, toidx);
  api_checkvalidindex(L, to);
  setobj(L, to, fr);
  if (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(L->ci->func), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API void NameDef(lua_pushvalue) (NameDef(lua_State) *L, int idx) {
  lua_lock(L);
  setobj2s(L, L->top, index2addr(L, idx));
  api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API int NameDef(lua_type) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  return (isvalid(o) ? ttnov(o) : LUA_TNONE);
}


LUA_API const char *NameDef(lua_typename) (NameDef(lua_State) *L, int t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTAGS, "invalid tag");
  return ttypename(t);
}


LUA_API int NameDef(lua_iscfunction) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  return (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API int NameDef(lua_isinteger) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  return ttisinteger(o);
}


LUA_API int NameDef(lua_isnumber) (NameDef(lua_State) *L, int idx) {
  NameDef(lua_Number) n;
  const NameDef(TValue) *o = index2addr(L, idx);
  return tonumber(o, &n);
}


LUA_API int NameDef(lua_isstring) (NameDef(lua_State) *L, int idx) {
  const NameDef(TValue) *o = index2addr(L, idx);
  return (ttisstring(o) || cvt2str(o));
}


LUA_API int NameDef(lua_isuserdata) (NameDef(lua_State) *L, int idx) {
  const NameDef(TValue) *o = index2addr(L, idx);
  return (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API int NameDef(lua_rawequal) (NameDef(lua_State) *L, int index1, int index2) {
  NameDef(StkId) o1 = index2addr(L, index1);
  NameDef(StkId) o2 = index2addr(L, index2);
  return (isvalid(o1) && isvalid(o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API void NameDef(lua_arith) (NameDef(lua_State) *L, int op) {
  lua_lock(L);
  if (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checknelems(L, 2);  /* all other operations expect two operands */
  else {  /* for unary operations, add fake 2nd operand */
    api_checknelems(L, 1);
    setobjs2s(L, L->top, L->top - 1);
    api_incr_top(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  NameDef(luaO_arith)(L, op, L->top - 2, L->top - 1, L->top - 2);
  L->top--;  /* remove second operand */
  lua_unlock(L);
}


LUA_API int NameDef(lua_compare) (NameDef(lua_State) *L, int index1, int index2, int op) {
  NameDef(StkId) o1, o2;
  int i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2addr(L, index1);
  o2 = index2addr(L, index2);
  if (isvalid(o1) && isvalid(o2)) {
    switch (op) {
      case LUA_OPEQ: i = NameDef(luaV_equalobj)(L, o1, o2); break;
      case LUA_OPLT: i = NameDef(luaV_lessthan)(L, o1, o2); break;
      case LUA_OPLE: i = NameDef(luaV_lessequal)(L, o1, o2); break;
      default: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  return i;
}


LUA_API size_t NameDef(lua_stringtonumber) (NameDef(lua_State) *L, const char *s) {
  size_t sz = NameDef(luaO_str2num)(s, L->top);
  if (sz != 0)
    api_incr_top(L);
  return sz;
}


LUA_API NameDef(lua_Number) NameDef(lua_tonumberx) (NameDef(lua_State) *L, int idx, int *pisnum) {
  NameDef(lua_Number) n;
  const NameDef(TValue) *o = index2addr(L, idx);
  int isnum = tonumber(o, &n);
  if (!isnum)
    n = 0;  /* call to 'tonumber' may change 'n' even if it fails */
  if (pisnum) *pisnum = isnum;
  return n;
}


LUA_API NameDef(lua_Integer) NameDef(lua_tointegerx) (NameDef(lua_State) *L, int idx, int *pisnum) {
  NameDef(lua_Integer) res;
  const NameDef(TValue) *o = index2addr(L, idx);
  int isnum = tointeger(o, &res);
  if (!isnum)
    res = 0;  /* call to 'tointeger' may change 'n' even if it fails */
  if (pisnum) *pisnum = isnum;
  return res;
}


LUA_API int NameDef(lua_toboolean) (NameDef(lua_State) *L, int idx) {
  const NameDef(TValue) *o = index2addr(L, idx);
  return !l_isfalse(o);
}


LUA_API const char *NameDef(lua_tolstring) (NameDef(lua_State) *L, int idx, size_t *len) {
  NameDef(StkId) o = index2addr(L, idx);
  if (!ttisstring(o)) {
    if (!cvt2str(o)) {  /* not convertible? */
      if (len != NULL) *len = 0;
      return NULL;
    }
    lua_lock(L);  /* 'luaO_tostring' may create a new string */
    luaC_checkGC(L);
    o = index2addr(L, idx);  /* previous call may reallocate the stack */
    NameDef(luaO_tostring)(L, o);
    lua_unlock(L);
  }
  if (len != NULL)
    *len = vslen(o);
  return svalue(o);
}


LUA_API size_t NameDef(lua_rawlen) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  switch (ttype(o)) {
    case LUA_TSHRSTR: return tsvalue(o)->shrlen;
    case LUA_TLNGSTR: return tsvalue(o)->u.lnglen;
    case LUA_TUSERDATA: return uvalue(o)->len;
    case LUA_TTABLE: return NameDef(luaH_getn)(hvalue(o));
    default: return 0;
  }
}


LUA_API NameDef(lua_CFunction) NameDef(lua_tocfunction) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  if (ttislcf(o)) return fvalue(o);
  else if (ttisCclosure(o))
    return clCvalue(o)->f;
  else return NULL;  /* not a C function */
}


LUA_API void *NameDef(lua_touserdata) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  switch (ttnov(o)) {
    case LUA_TUSERDATA: return getudatamem(uvalue(o));
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}


LUA_API NameDef(lua_State) *NameDef(lua_tothread) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}


LUA_API const void *NameDef(lua_topointer) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o = index2addr(L, idx);
  switch (ttype(o)) {
    case LUA_TTABLE: return hvalue(o);
    case LUA_TLCL: return clLvalue(o);
    case LUA_TCCL: return clCvalue(o);
    case LUA_TLCF: return cast(void *, cast(size_t, fvalue(o)));
    case LUA_TTHREAD: return thvalue(o);
    case LUA_TUSERDATA: return getudatamem(uvalue(o));
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}



/*
** push functions (C -> stack)
*/


LUA_API void NameDef(lua_pushnil) (NameDef(lua_State) *L) {
  lua_lock(L);
  setnilvalue(L->top);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void NameDef(lua_pushnumber) (NameDef(lua_State) *L, NameDef(lua_Number) n) {
  lua_lock(L);
  setfltvalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void NameDef(lua_pushinteger) (NameDef(lua_State) *L, NameDef(lua_Integer) n) {
  lua_lock(L);
  setivalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API const char *NameDef(lua_pushlstring) (NameDef(lua_State) *L, const char *s, size_t len) {
  NameDef(TString) *ts;
  lua_lock(L);
  luaC_checkGC(L);
  ts = NameDef(luaS_newlstr)(L, s, len);
  setsvalue2s(L, L->top, ts);
  api_incr_top(L);
  lua_unlock(L);
  return getstr(ts);
}


LUA_API const char *NameDef(lua_pushstring) (NameDef(lua_State) *L, const char *s) {
  lua_lock(L);
  if (s == NULL)
    setnilvalue(L->top);
  else {
    NameDef(TString) *ts;
    luaC_checkGC(L);
    ts = NameDef(luaS_new)(L, s);
    setsvalue2s(L, L->top, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_top(L);
  lua_unlock(L);
  return s;
}


LUA_API const char *NameDef(lua_pushvfstring) (NameDef(lua_State) *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  luaC_checkGC(L);
  ret = NameDef(luaO_pushvfstring)(L, fmt, argp);
  lua_unlock(L);
  return ret;
}


LUA_API const char *NameDef(lua_pushfstring) (NameDef(lua_State) *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  lua_lock(L);
  luaC_checkGC(L);
  va_start(argp, fmt);
  ret = NameDef(luaO_pushvfstring)(L, fmt, argp);
  va_end(argp);
  lua_unlock(L);
  return ret;
}


LUA_API void NameDef(lua_pushcclosure) (NameDef(lua_State) *L, NameDef(lua_CFunction) fn, int n) {
  lua_lock(L);
  if (n == 0) {
    setfvalue(L->top, fn);
  }
  else {
    NameDef(CClosure) *cl;
    api_checknelems(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    luaC_checkGC(L);
    cl = NameDef(luaF_newCclosure)(L, n);
    cl->f = fn;
    L->top -= n;
    while (n--) {
      setobj2n(L, &cl->upvalue[n], L->top + n);
      /* does not need barrier because closure is white */
    }
    setclCvalue(L, L->top, cl);
  }
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void NameDef(lua_pushboolean) (NameDef(lua_State) *L, int b) {
  lua_lock(L);
  setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void NameDef(lua_pushlightuserdata) (NameDef(lua_State) *L, void *p) {
  lua_lock(L);
  setpvalue(L->top, p);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int NameDef(lua_pushthread) (NameDef(lua_State) *L) {
  lua_lock(L);
  setthvalue(L, L->top, L);
  api_incr_top(L);
  lua_unlock(L);
  return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/


LUA_API int NameDef(lua_getglobal) (NameDef(lua_State) *L, const char *name) {
  NameDef(Table) *reg = hvalue(&G(L)->l_registry);
  const NameDef(TValue) *gt;  /* global table */
  lua_lock(L);
  gt = NameDef(luaH_getint)(reg, LUA_RIDX_GLOBALS);
  setsvalue2s(L, L->top, NameDef(luaS_new)(L, name));
  api_incr_top(L);
  NameDef(luaV_gettable)(L, gt, L->top - 1, L->top - 1);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API int NameDef(lua_gettable) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) t;
  lua_lock(L);
  t = index2addr(L, idx);
  NameDef(luaV_gettable)(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API int NameDef(lua_getfield) (NameDef(lua_State) *L, int idx, const char *k) {
  NameDef(StkId) t;
  lua_lock(L);
  t = index2addr(L, idx);
  setsvalue2s(L, L->top, NameDef(luaS_new)(L, k));
  api_incr_top(L);
  NameDef(luaV_gettable)(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API int NameDef(lua_geti) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n) {
  NameDef(StkId) t;
  lua_lock(L);
  t = index2addr(L, idx);
  setivalue(L->top, n);
  api_incr_top(L);
  NameDef(luaV_gettable)(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API int NameDef(lua_rawget) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) t;
  lua_lock(L);
  t = index2addr(L, idx);
  api_check(L, ttistable(t), "table expected");
  setobj2s(L, L->top - 1, NameDef(luaH_get)(hvalue(t), L->top - 1));
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API int NameDef(lua_rawgeti) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n) {
  NameDef(StkId) t;
  lua_lock(L);
  t = index2addr(L, idx);
  api_check(L, ttistable(t), "table expected");
  setobj2s(L, L->top, NameDef(luaH_getint)(hvalue(t), n));
  api_incr_top(L);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API int NameDef(lua_rawgetp) (NameDef(lua_State) *L, int idx, const void *p) {
  NameDef(StkId) t;
  NameDef(TValue) k;
  lua_lock(L);
  t = index2addr(L, idx);
  api_check(L, ttistable(t), "table expected");
  setpvalue(&k, cast(void *, p));
  setobj2s(L, L->top, NameDef(luaH_get)(hvalue(t), &k));
  api_incr_top(L);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


LUA_API void NameDef(lua_createtable) (NameDef(lua_State) *L, int narray, int nrec) {
  NameDef(Table) *t;
  lua_lock(L);
  luaC_checkGC(L);
  t = NameDef(luaH_new)(L);
  sethvalue(L, L->top, t);
  api_incr_top(L);
  if (narray > 0 || nrec > 0)
    NameDef(luaH_resize)(L, t, narray, nrec);
  lua_unlock(L);
}


LUA_API int NameDef(lua_getmetatable) (NameDef(lua_State) *L, int objindex) {
  const NameDef(TValue) *obj;
  NameDef(Table) *mt;
  int res = 0;
  lua_lock(L);
  obj = index2addr(L, objindex);
  switch (ttnov(obj)) {
    case LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      break;
    default:
      mt = G(L)->mt[ttnov(obj)];
      break;
  }
  if (mt != NULL) {
    sethvalue(L, L->top, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}


LUA_API int NameDef(lua_getuservalue) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o;
  lua_lock(L);
  o = index2addr(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  getuservalue(L, uvalue(o), L->top);
  api_incr_top(L);
  lua_unlock(L);
  return ttnov(L->top - 1);
}


/*
** set functions (stack -> Lua)
*/


LUA_API void NameDef(lua_setglobal) (NameDef(lua_State) *L, const char *name) {
  NameDef(Table) *reg = hvalue(&G(L)->l_registry);
  const NameDef(TValue) *gt;  /* global table */
  lua_lock(L);
  api_checknelems(L, 1);
  gt = NameDef(luaH_getint)(reg, LUA_RIDX_GLOBALS);
  setsvalue2s(L, L->top, NameDef(luaS_new)(L, name));
  api_incr_top(L);
  NameDef(luaV_settable)(L, gt, L->top - 1, L->top - 2);
  L->top -= 2;  /* pop value and key */
  lua_unlock(L);
}


LUA_API void NameDef(lua_settable) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2addr(L, idx);
  NameDef(luaV_settable)(L, t, L->top - 2, L->top - 1);
  L->top -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API void NameDef(lua_setfield) (NameDef(lua_State) *L, int idx, const char *k) {
  NameDef(StkId) t;
  lua_lock(L);
  api_checknelems(L, 1);
  t = index2addr(L, idx);
  setsvalue2s(L, L->top, NameDef(luaS_new)(L, k));
  api_incr_top(L);
  NameDef(luaV_settable)(L, t, L->top - 1, L->top - 2);
  L->top -= 2;  /* pop value and key */
  lua_unlock(L);
}


LUA_API void NameDef(lua_seti) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n) {
  NameDef(StkId) t;
  lua_lock(L);
  api_checknelems(L, 1);
  t = index2addr(L, idx);
  setivalue(L->top, n);
  api_incr_top(L);
  NameDef(luaV_settable)(L, t, L->top - 1, L->top - 2);
  L->top -= 2;  /* pop value and key */
  lua_unlock(L);
}


LUA_API void NameDef(lua_rawset) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o;
  NameDef(Table) *t;
  lua_lock(L);
  api_checknelems(L, 2);
  o = index2addr(L, idx);
  api_check(L, ttistable(o), "table expected");
  t = hvalue(o);
  setobj2t(L, NameDef(luaH_set)(L, t, L->top-2), L->top-1);
  invalidateTMcache(t);
  luaC_barrierback(L, t, L->top-1);
  L->top -= 2;
  lua_unlock(L);
}


LUA_API void NameDef(lua_rawseti) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n) {
  NameDef(StkId) o;
  NameDef(Table) *t;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2addr(L, idx);
  api_check(L, ttistable(o), "table expected");
  t = hvalue(o);
  NameDef(luaH_setint)(L, t, n, L->top - 1);
  luaC_barrierback(L, t, L->top-1);
  L->top--;
  lua_unlock(L);
}


LUA_API void NameDef(lua_rawsetp) (NameDef(lua_State) *L, int idx, const void *p) {
  NameDef(StkId) o;
  NameDef(Table) *t;
  NameDef(TValue) k;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2addr(L, idx);
  api_check(L, ttistable(o), "table expected");
  t = hvalue(o);
  setpvalue(&k, cast(void *, p));
  setobj2t(L, NameDef(luaH_set)(L, t, &k), L->top - 1);
  luaC_barrierback(L, t, L->top - 1);
  L->top--;
  lua_unlock(L);
}


LUA_API int NameDef(lua_setmetatable) (NameDef(lua_State) *L, int objindex) {
  NameDef(TValue) *obj;
  NameDef(Table) *mt;
  lua_lock(L);
  api_checknelems(L, 1);
  obj = index2addr(L, objindex);
  if (ttisnil(L->top - 1))
    mt = NULL;
  else {
    api_check(L, ttistable(L->top - 1), "table expected");
    mt = hvalue(L->top - 1);
  }
  switch (ttnov(obj)) {
    case LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      if (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        NameDef(luaC_checkfinalizer)(L, gcvalue(obj), mt);
      }
      break;
    }
    case LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      if (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        NameDef(luaC_checkfinalizer)(L, gcvalue(obj), mt);
      }
      break;
    }
    default: {
      G(L)->mt[ttnov(obj)] = mt;
      break;
    }
  }
  L->top--;
  lua_unlock(L);
  return 1;
}


LUA_API void NameDef(lua_setuservalue) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2addr(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  setuservalue(L, uvalue(o), L->top - 1);
  luaC_barrier(L, gcvalue(o), L->top - 1);
  L->top--;
  lua_unlock(L);
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)), \
	"results from function overflow current stack size")


LUA_API void NameDef(lua_callk) (NameDef(lua_State) *L, int nargs, int nresults,
                        NameDef(lua_KContext) ctx, NameDef(lua_KFunction) k) {
  NameDef(StkId) func;
  lua_lock(L);
  api_check(L, k == NULL || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checknelems(L, nargs+1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->top - (nargs+1);
  if (k != NULL && L->nny == 0) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    NameDef(luaD_call)(L, func, nresults, 1);  /* do the call */
  }
  else  /* no continuation or no yieldable */
    NameDef(luaD_call)(L, func, nresults, 0);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct NameDef(CallS) {  /* data to 'f_call' */
  NameDef(StkId) func;
  int nresults;
};


static void f_call (NameDef(lua_State) *L, void *ud) {
  struct NameDef(CallS) *c = cast(struct NameDef(CallS) *, ud);
  NameDef(luaD_call)(L, c->func, c->nresults, 0);
}



LUA_API int NameDef(lua_pcallk) (NameDef(lua_State) *L, int nargs, int nresults, int errfunc,
                        NameDef(lua_KContext) ctx, NameDef(lua_KFunction) k) {
  struct NameDef(CallS) c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == NULL || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checknelems(L, nargs+1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    NameDef(StkId) o = index2addr(L, errfunc);
    api_checkstackindex(L, errfunc, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  if (k == NULL || L->nny > 0) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = NameDef(luaD_pcall)(L, f_call, &c, savestack(L, c.func), func);
  }
  else {  /* prepare continuation (call is already protected by 'resume') */
    NameDef(CallInfo) *ci = L->ci;
    ci->u.c.k = k;  /* save continuation */
    ci->u.c.ctx = ctx;  /* save context */
    /* save information for error recovery */
    ci->extra = savestack(L, c.func);
    ci->u.c.old_errfunc = L->errfunc;
    L->errfunc = func;
    setoah(ci->callstatus, L->allowhook);  /* save value of 'allowhook' */
    ci->callstatus |= CIST_YPCALL;  /* function can do error recovery */
    NameDef(luaD_call)(L, c.func, nresults, 1);  /* do the call */
    ci->callstatus &= ~CIST_YPCALL;
    L->errfunc = ci->u.c.old_errfunc;
    status = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


LUA_API int NameDef(lua_load) (NameDef(lua_State) *L, NameDef(lua_Reader) reader, void *data,
                      const char *chunkname, const char *mode) {
  NameDef(ZIO) z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  NameDef(luaZ_init)(L, &z, reader, data);
  status = NameDef(luaD_protectedparser)(L, &z, chunkname, mode);
  if (status == LUA_OK) {  /* no errors? */
    NameDef(LClosure) *f = clLvalue(L->top - 1);  /* get newly created function */
    if (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      NameDef(Table) *reg = hvalue(&G(L)->l_registry);
      const NameDef(TValue) *gt = NameDef(luaH_getint)(reg, LUA_RIDX_GLOBALS);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v, gt);
      luaC_upvalbarrier(L, f->upvals[0]);
    }
  }
  lua_unlock(L);
  return status;
}


LUA_API int NameDef(lua_dump) (NameDef(lua_State) *L, NameDef(lua_Writer) writer, void *data, int strip) {
  int status;
  NameDef(TValue) *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o))
    status = NameDef(luaU_dump)(L, getproto(o), writer, data, strip);
  else
    status = 1;
  lua_unlock(L);
  return status;
}


LUA_API int NameDef(lua_status) (NameDef(lua_State) *L) {
  return L->status;
}


/*
** Garbage-collection function
*/

LUA_API int NameDef(lua_gc) (NameDef(lua_State) *L, int what, int data) {
  int res = 0;
  NameDef(global_State) *g;
  lua_lock(L);
  g = G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->gcrunning = 0;
      break;
    }
    case LUA_GCRESTART: {
      NameDef(luaE_setdebt)(g, 0);
      g->gcrunning = 1;
      break;
    }
    case LUA_GCCOLLECT: {
      NameDef(luaC_fullgc)(L, 0);
      break;
    }
    case LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      NameDef(l_mem) debt = 1;  /* =1 to signal that it did an actual step */
      int oldrunning = g->gcrunning;
      g->gcrunning = 1;  /* allow GC to run */
      if (data == 0) {
        NameDef(luaE_setdebt)(g, -GCSTEPSIZE);  /* to do a "small" step */
        NameDef(luaC_step)(L);
      }
      else {  /* add 'data' to total debt */
        debt = cast(NameDef(l_mem), data) * 1024 + g->GCdebt;
        NameDef(luaE_setdebt)(g, debt);
        luaC_checkGC(L);
      }
      g->gcrunning = oldrunning;  /* restore previous state */
      if (debt > 0 && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      if (data < 40) data = 40;  /* avoid ridiculous low values (and 0) */
      g->gcstepmul = data;
      break;
    }
    case LUA_GCISRUNNING: {
      res = g->gcrunning;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/


LUA_API int NameDef(lua_error) (NameDef(lua_State) *L) {
  lua_lock(L);
  api_checknelems(L, 1);
  NameDef(luaG_errormsg)(L);
  /* code unreachable; will unlock when control actually leaves the kernel */
  return 0;  /* to avoid warnings */
}


LUA_API int NameDef(lua_next) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) t;
  int more;
  lua_lock(L);
  t = index2addr(L, idx);
  api_check(L, ttistable(t), "table expected");
  more = NameDef(luaH_next)(L, hvalue(t), L->top - 1);
  if (more) {
    api_incr_top(L);
  }
  else  /* no more elements */
    L->top -= 1;  /* remove key */
  lua_unlock(L);
  return more;
}


LUA_API void NameDef(lua_concat) (NameDef(lua_State) *L, int n) {
  lua_lock(L);
  api_checknelems(L, n);
  if (n >= 2) {
    luaC_checkGC(L);
    NameDef(luaV_concat)(L, n);
  }
  else if (n == 0) {  /* push empty string */
    setsvalue2s(L, L->top, NameDef(luaS_newlstr)(L, "", 0));
    api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  lua_unlock(L);
}


LUA_API void NameDef(lua_len) (NameDef(lua_State) *L, int idx) {
  NameDef(StkId) t;
  lua_lock(L);
  t = index2addr(L, idx);
  NameDef(luaV_objlen)(L, L->top, t);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API NameDef(lua_Alloc) NameDef(lua_getallocf) (NameDef(lua_State) *L, void **ud) {
  NameDef(lua_Alloc) f;
  lua_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  return f;
}


LUA_API void NameDef(lua_setallocf) (NameDef(lua_State) *L, NameDef(lua_Alloc) f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


LUA_API void *NameDef(lua_newuserdata) (NameDef(lua_State) *L, size_t size) {
  NameDef(Udata) *u;
  lua_lock(L);
  luaC_checkGC(L);
  u = NameDef(luaS_newudata)(L, size);
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
  return getudatamem(u);
}



static const char *aux_upvalue (NameDef(StkId) fi, int n, NameDef(TValue) **val,
                                NameDef(CClosure) **owner, NameDef(UpVal) **uv) {
  switch (ttype(fi)) {
    case LUA_TCCL: {  /* C closure */
      NameDef(CClosure) *f = clCvalue(fi);
      if (!(1 <= n && n <= f->nupvalues)) return NULL;
      *val = &f->upvalue[n-1];
      if (owner) *owner = f;
      return "";
    }
    case LUA_TLCL: {  /* Lua closure */
      NameDef(LClosure) *f = clLvalue(fi);
      NameDef(TString) *name;
      NameDef(Proto) *p = f->p;
      if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
      *val = f->upvals[n-1]->v;
      if (uv) *uv = f->upvals[n - 1];
      name = p->upvalues[n-1].name;
      return (name == NULL) ? "(*no name)" : getstr(name);
    }
    default: return NULL;  /* not a closure */
  }
}


LUA_API const char *NameDef(lua_getupvalue) (NameDef(lua_State) *L, int funcindex, int n) {
  const char *name;
  NameDef(TValue) *val = NULL;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2addr(L, funcindex), n, &val, NULL, NULL);
  if (name) {
    setobj2s(L, L->top, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}


LUA_API const char *NameDef(lua_setupvalue) (NameDef(lua_State) *L, int funcindex, int n) {
  const char *name;
  NameDef(TValue) *val = NULL;  /* to avoid warnings */
  NameDef(CClosure) *owner = NULL;
  NameDef(UpVal) *uv = NULL;
  NameDef(StkId) fi;
  lua_lock(L);
  fi = index2addr(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val, &owner, &uv);
  if (name) {
    L->top--;
    setobj(L, val, L->top);
    if (owner) { luaC_barrier(L, owner, L->top); }
    else if (uv) { luaC_upvalbarrier(L, uv); }
  }
  lua_unlock(L);
  return name;
}


static NameDef(UpVal) **getupvalref (NameDef(lua_State) *L, int fidx, int n, NameDef(LClosure) **pf) {
  NameDef(LClosure) *f;
  NameDef(StkId) fi = index2addr(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  api_check(L, (1 <= n && n <= f->p->sizeupvalues), "invalid upvalue index");
  if (pf) *pf = f;
  return &f->upvals[n - 1];  /* get its upvalue pointer */
}


LUA_API void *NameDef(lua_upvalueid) (NameDef(lua_State) *L, int fidx, int n) {
  NameDef(StkId) fi = index2addr(L, fidx);
  switch (ttype(fi)) {
    case LUA_TLCL: {  /* lua closure */
      return *getupvalref(L, fidx, n, NULL);
    }
    case LUA_TCCL: {  /* C closure */
      NameDef(CClosure) *f = clCvalue(fi);
      api_check(L, 1 <= n && n <= f->nupvalues, "invalid upvalue index");
      return &f->upvalue[n - 1];
    }
    default: {
      api_check(L, 0, "closure expected");
      return NULL;
    }
  }
}


LUA_API void NameDef(lua_upvaluejoin) (NameDef(lua_State) *L, int fidx1, int n1,
                                            int fidx2, int n2) {
  NameDef(LClosure) *f1;
  NameDef(UpVal) **up1 = getupvalref(L, fidx1, n1, &f1);
  NameDef(UpVal) **up2 = getupvalref(L, fidx2, n2, NULL);
  NameDef(luaC_upvdeccount)(L, *up1);
  *up1 = *up2;
  (*up1)->refcount++;
  if (upisopen(*up1)) (*up1)->u.open.touched = 1;
  luaC_upvalbarrier(L, *up1);
}


