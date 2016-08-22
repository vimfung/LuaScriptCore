/*
** $Id: lvm.c,v 2.268 2016/02/05 19:59:14 roberto Exp $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#define lvm_c
#define LUA_CORE

#include "lprefix.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"


/* limit for table tag-method chains (to avoid loops) */
#define MAXTAGLOOP	2000



/*
** 'l_intfitsf' checks whether a given integer can be converted to a
** float without rounding. Used in comparisons. Left undefined if
** all integers fit in a float precisely.
*/
#if !defined(l_intfitsf)

/* number of bits in the mantissa of a float */
#define NBM		(l_mathlim(MANT_DIG))

/*
** Check whether some integers may not fit in a float, that is, whether
** (maxinteger >> NBM) > 0 (that implies (1 << NBM) <= maxinteger).
** (The shifts are done in parts to avoid shifting by more than the size
** of an integer. In a worst case, NBM == 113 for long double and
** sizeof(integer) == 32.)
*/
#if ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

#define l_intfitsf(i)  \
  (-((NameDef(lua_Integer))1 << NBM) <= (i) && (i) <= ((NameDef(lua_Integer))1 << NBM))

#endif

#endif



/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
int NameDef(luaV_tonumber_) (const NameDef(TValue) *obj, NameDef(lua_Number) *n) {
  NameDef(TValue) v;
  if (ttisinteger(obj)) {
    *n = cast_num(ivalue(obj));
    return 1;
  }
  else if (cvt2num(obj) &&  /* string convertible to number? */
            NameDef(luaO_str2num)(svalue(obj), &v) == vslen(obj) + 1) {
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    return 1;
  }
  else
    return 0;  /* conversion failed */
}


/*
** try to convert a value to an integer, rounding according to 'mode':
** mode == 0: accepts only integral values
** mode == 1: takes the floor of the number
** mode == 2: takes the ceil of the number
*/
int NameDef(luaV_tointeger) (const NameDef(TValue) *obj, NameDef(lua_Integer) *p, int mode) {
  NameDef(TValue) v;
 again:
  if (ttisfloat(obj)) {
    NameDef(lua_Number) n = fltvalue(obj);
    NameDef(lua_Number) f = l_floor(n);
    if (n != f) {  /* not an integral value? */
      if (mode == 0) return 0;  /* fails if mode demands integral value */
      else if (mode > 1)  /* needs ceil? */
        f += 1;  /* convert floor to ceil (remember: n != f) */
    }
    return lua_numbertointeger(f, p);
  }
  else if (ttisinteger(obj)) {
    *p = ivalue(obj);
    return 1;
  }
  else if (cvt2num(obj) &&
            NameDef(luaO_str2num)(svalue(obj), &v) == vslen(obj) + 1) {
    obj = &v;
    goto again;  /* convert result from 'luaO_str2num' to an integer */
  }
  return 0;  /* conversion failed */
}


/*
** Try to convert a 'for' limit to an integer, preserving the
** semantics of the loop.
** (The following explanation assumes a non-negative step; it is valid
** for negative steps mutatis mutandis.)
** If the limit can be converted to an integer, rounding down, that is
** it.
** Otherwise, check whether the limit can be converted to a number.  If
** the number is too large, it is OK to set the limit as LUA_MAXINTEGER,
** which means no limit.  If the number is too negative, the loop
** should not run, because any initial integer value is larger than the
** limit. So, it sets the limit to LUA_MININTEGER. 'stopnow' corrects
** the extreme case when the initial value is LUA_MININTEGER, in which
** case the LUA_MININTEGER limit would still run the loop once.
*/
static int forlimit (const NameDef(TValue) *obj, NameDef(lua_Integer) *p, NameDef(lua_Integer) step,
                     int *stopnow) {
  *stopnow = 0;  /* usually, let loops run */
  if (!NameDef(luaV_tointeger)(obj, p, (step < 0 ? 2 : 1))) {  /* not fit in integer? */
    NameDef(lua_Number) n;  /* try to convert to float */
    if (!tonumber(obj, &n)) /* cannot convert to float? */
      return 0;  /* not a number */
    if (luai_numlt(0, n)) {  /* if true, float is larger than max integer */
      *p = LUA_MAXINTEGER;
      if (step < 0) *stopnow = 1;
    }
    else {  /* float is smaller than min integer */
      *p = LUA_MININTEGER;
      if (step >= 0) *stopnow = 1;
    }
  }
  return 1;
}


/*
** Finish the table access 'val = t[key]'.
** if 'slot' is NULL, 't' is not a table; otherwise, 'slot' points to
** t[k] entry (which must be nil).
*/
void NameDef(luaV_finishget) (NameDef(lua_State) *L, const NameDef(TValue) *t, NameDef(TValue) *key, NameDef(StkId) val,
                      const NameDef(TValue) *slot) {
  int loop;  /* counter to avoid infinite loops */
  const NameDef(TValue) *tm;  /* metamethod */
  for (loop = 0; loop < MAXTAGLOOP; loop++) {
    if (slot == NULL) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      tm = NameDef(luaT_gettmbyobj)(L, t, NameDef(TM_INDEX));
      if (ttisnil(tm))
        NameDef(luaG_typeerror)(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    else {  /* 't' is a table */
      lua_assert(ttisnil(slot));
      tm = fasttm(L, hvalue(t)->metatable, NameDef(TM_INDEX));  /* table's metamethod */
      if (tm == NULL) {  /* no metamethod? */
        setnilvalue(val);  /* result is nil */
        return;
      }
      /* else will try the metamethod */
    }
    if (ttisfunction(tm)) {  /* is metamethod a function? */
      NameDef(luaT_callTM)(L, tm, t, key, val, 1);  /* call it */
      return;
    }
    t = tm;  /* else try to access 'tm[key]' */
    if (luaV_fastget(L,t,key,slot,NameDef(luaH_get))) {  /* fast track? */
      setobj2s(L, val, slot);  /* done */
      return;
    }
    /* else repeat (tail call 'luaV_finishget') */
  }
  NameDef(luaG_runerror)(L, "'__index' chain too long; possible loop");
}


/*
** Finish a table assignment 't[key] = val'.
** If 'slot' is NULL, 't' is not a table.  Otherwise, 'slot' points
** to the entry 't[key]', or to 'luaO_nilobject' if there is no such
** entry.  (The value at 'slot' must be nil, otherwise 'luaV_fastset'
** would have done the job.)
*/
void NameDef(luaV_finishset) (NameDef(lua_State) *L, const NameDef(TValue) *t, NameDef(TValue) *key,
                     NameDef(StkId) val, const NameDef(TValue) *slot) {
  int loop;  /* counter to avoid infinite loops */
  for (loop = 0; loop < MAXTAGLOOP; loop++) {
    const NameDef(TValue) *tm;  /* '__newindex' metamethod */
    if (slot != NULL) {  /* is 't' a table? */
      NameDef(Table) *h = hvalue(t);  /* save 't' table */
      lua_assert(ttisnil(slot));  /* old value must be nil */
      tm = fasttm(L, h->metatable, NameDef(TM_NEWINDEX));  /* get metamethod */
      if (tm == NULL) {  /* no metamethod? */
        if (slot == luaO_nilobject)  /* no previous entry? */
          slot = NameDef(luaH_newkey)(L, h, key);  /* create one */
        /* no metamethod and (now) there is an entry with given key */
        setobj2t(L, cast(NameDef(TValue) *, slot), val);  /* set its new value */
        invalidateTMcache(h);
        luaC_barrierback(L, h, val);
        return;
      }
      /* else will try the metamethod */
    }
    else {  /* not a table; check metamethod */
      if (ttisnil(tm = NameDef(luaT_gettmbyobj)(L, t, NameDef(TM_NEWINDEX))))
        NameDef(luaG_typeerror)(L, t, "index");
    }
    /* try the metamethod */
    if (ttisfunction(tm)) {
      NameDef(luaT_callTM)(L, tm, t, key, val, 0);
      return;
    }
    t = tm;  /* else repeat assignment over 'tm' */
    if (luaV_fastset(L, t, key, slot, NameDef(luaH_get), val))
      return;  /* done */
    /* else loop */
  }
  NameDef(luaG_runerror)(L, "'__newindex' chain too long; possible loop");
}


/*
** Compare two strings 'ls' x 'rs', returning an integer smaller-equal-
** -larger than zero if 'ls' is smaller-equal-larger than 'rs'.
** The code is a little tricky because it allows '\0' in the strings
** and it uses 'strcoll' (to respect locales) for each segments
** of the strings.
*/
static int l_strcmp (const NameDef(TString) *ls, const NameDef(TString) *rs) {
  const char *l = getstr(ls);
  size_t ll = tsslen(ls);
  const char *r = getstr(rs);
  size_t lr = tsslen(rs);
  for (;;) {  /* for each segment */
    int temp = strcoll(l, r);
    if (temp != 0)  /* not equal? */
      return temp;  /* done */
    else {  /* strings are equal up to a '\0' */
      size_t len = strlen(l);  /* index of first '\0' in both strings */
      if (len == lr)  /* 'rs' is finished? */
        return (len == ll) ? 0 : 1;  /* check 'ls' */
      else if (len == ll)  /* 'ls' is finished? */
        return -1;  /* 'ls' is smaller than 'rs' ('rs' is not finished) */
      /* both strings longer than 'len'; go on comparing after the '\0' */
      len++;
      l += len; ll -= len; r += len; lr -= len;
    }
  }
}


/*
** Check whether integer 'i' is less than float 'f'. If 'i' has an
** exact representation as a float ('l_intfitsf'), compare numbers as
** floats. Otherwise, if 'f' is outside the range for integers, result
** is trivial. Otherwise, compare them as integers. (When 'i' has no
** float representation, either 'f' is "far away" from 'i' or 'f' has
** no precision left for a fractional part; either way, how 'f' is
** truncated is irrelevant.) When 'f' is NaN, comparisons must result
** in false.
*/
static int LTintfloat (NameDef(lua_Integer) i, NameDef(lua_Number) f) {
#if defined(l_intfitsf)
  if (!l_intfitsf(i)) {
    if (f >= -cast_num(LUA_MININTEGER))  /* -minint == maxint + 1 */
      return 1;  /* f >= maxint + 1 > i */
    else if (f > cast_num(LUA_MININTEGER))  /* minint < f <= maxint ? */
      return (i < cast(NameDef(lua_Integer), f));  /* compare them as integers */
    else  /* f <= minint <= i (or 'f' is NaN)  -->  not(i < f) */
      return 0;
  }
#endif
  return luai_numlt(cast_num(i), f);  /* compare them as floats */
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
static int LEintfloat (NameDef(lua_Integer) i, NameDef(lua_Number) f) {
#if defined(l_intfitsf)
  if (!l_intfitsf(i)) {
    if (f >= -cast_num(LUA_MININTEGER))  /* -minint == maxint + 1 */
      return 1;  /* f >= maxint + 1 > i */
    else if (f >= cast_num(LUA_MININTEGER))  /* minint <= f <= maxint ? */
      return (i <= cast(NameDef(lua_Integer), f));  /* compare them as integers */
    else  /* f < minint <= i (or 'f' is NaN)  -->  not(i <= f) */
      return 0;
  }
#endif
  return luai_numle(cast_num(i), f);  /* compare them as floats */
}


/*
** Return 'l < r', for numbers.
*/
static int LTnum (const NameDef(TValue) *l, const NameDef(TValue) *r) {
  if (ttisinteger(l)) {
    NameDef(lua_Integer) li = ivalue(l);
    if (ttisinteger(r))
      return li < ivalue(r);  /* both are integers */
    else  /* 'l' is int and 'r' is float */
      return LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  else {
    NameDef(lua_Number) lf = fltvalue(l);  /* 'l' must be float */
    if (ttisfloat(r))
      return luai_numlt(lf, fltvalue(r));  /* both are float */
    else if (luai_numisnan(lf))  /* 'r' is int and 'l' is float */
      return 0;  /* NaN < i is always false */
    else  /* without NaN, (l < r)  <-->  not(r <= l) */
      return !LEintfloat(ivalue(r), lf);  /* not (r <= l) ? */
  }
}


/*
** Return 'l <= r', for numbers.
*/
static int LEnum (const NameDef(TValue) *l, const NameDef(TValue) *r) {
  if (ttisinteger(l)) {
    NameDef(lua_Integer) li = ivalue(l);
    if (ttisinteger(r))
      return li <= ivalue(r);  /* both are integers */
    else  /* 'l' is int and 'r' is float */
      return LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  else {
    NameDef(lua_Number) lf = fltvalue(l);  /* 'l' must be float */
    if (ttisfloat(r))
      return luai_numle(lf, fltvalue(r));  /* both are float */
    else if (luai_numisnan(lf))  /* 'r' is int and 'l' is float */
      return 0;  /*  NaN <= i is always false */
    else  /* without NaN, (l <= r)  <-->  not(r < l) */
      return !LTintfloat(ivalue(r), lf);  /* not (r < l) ? */
  }
}


/*
** Main operation less than; return 'l < r'.
*/
int NameDef(luaV_lessthan) (NameDef(lua_State) *L, const NameDef(TValue) *l, const NameDef(TValue) *r) {
  int res;
  if (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    return LTnum(l, r);
  else if (ttisstring(l) && ttisstring(r))  /* both are strings? */
    return l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  else if ((res = NameDef(luaT_callorderTM)(L, l, r, NameDef(TM_LT))) < 0)  /* no metamethod? */
    NameDef(luaG_ordererror)(L, l, r);  /* error */
  return res;
}


/*
** Main operation less than or equal to; return 'l <= r'. If it needs
** a metamethod and there is no '__le', try '__lt', based on
** l <= r iff !(r < l) (assuming a total order). If the metamethod
** yields during this substitution, the continuation has to know
** about it (to negate the result of r<l); bit CIST_LEQ in the call
** status keeps that information.
*/
int NameDef(luaV_lessequal) (NameDef(lua_State) *L, const NameDef(TValue) *l, const NameDef(TValue) *r) {
  int res;
  if (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    return LEnum(l, r);
  else if (ttisstring(l) && ttisstring(r))  /* both are strings? */
    return l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  else if ((res = NameDef(luaT_callorderTM)(L, l, r, NameDef(TM_LE))) >= 0)  /* try 'le' */
    return res;
  else {  /* try 'lt': */
    L->ci->callstatus |= CIST_LEQ;  /* mark it is doing 'lt' for 'le' */
    res = NameDef(luaT_callorderTM)(L, r, l, NameDef(TM_LT));
    L->ci->callstatus ^= CIST_LEQ;  /* clear mark */
    if (res < 0)
      NameDef(luaG_ordererror)(L, l, r);
    return !res;  /* result is negated */
  }
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
int NameDef(luaV_equalobj) (NameDef(lua_State) *L, const NameDef(TValue) *t1, const NameDef(TValue) *t2) {
  const NameDef(TValue) *tm;
  if (ttype(t1) != ttype(t2)) {  /* not the same variant? */
    if (ttnov(t1) != ttnov(t2) || ttnov(t1) != LUA_TNUMBER)
      return 0;  /* only numbers can be equal with different variants */
    else {  /* two numbers with different variants */
      NameDef(lua_Integer) i1, i2;  /* compare them as integers */
      return (tointeger(t1, &i1) && tointeger(t2, &i2) && i1 == i2);
    }
  }
  /* values have same type and same variant */
  switch (ttype(t1)) {
    case LUA_TNIL: return 1;
    case LUA_TNUMINT: return (ivalue(t1) == ivalue(t2));
    case LUA_TNUMFLT: return luai_numeq(fltvalue(t1), fltvalue(t2));
    case LUA_TBOOLEAN: return bvalue(t1) == bvalue(t2);  /* true must be 1 !! */
    case LUA_TLIGHTUSERDATA: return pvalue(t1) == pvalue(t2);
    case LUA_TLCF: return fvalue(t1) == fvalue(t2);
    case LUA_TSHRSTR: return eqshrstr(tsvalue(t1), tsvalue(t2));
    case LUA_TLNGSTR: return NameDef(luaS_eqlngstr)(tsvalue(t1), tsvalue(t2));
    case LUA_TUSERDATA: {
      if (uvalue(t1) == uvalue(t2)) return 1;
      else if (L == NULL) return 0;
      tm = fasttm(L, uvalue(t1)->metatable, NameDef(TM_EQ));
      if (tm == NULL)
        tm = fasttm(L, uvalue(t2)->metatable, NameDef(TM_EQ));
      break;  /* will try TM */
    }
    case LUA_TTABLE: {
      if (hvalue(t1) == hvalue(t2)) return 1;
      else if (L == NULL) return 0;
      tm = fasttm(L, hvalue(t1)->metatable, NameDef(TM_EQ));
      if (tm == NULL)
        tm = fasttm(L, hvalue(t2)->metatable, NameDef(TM_EQ));
      break;  /* will try TM */
    }
    default:
      return gcvalue(t1) == gcvalue(t2);
  }
  if (tm == NULL)  /* no TM? */
    return 0;  /* objects are different */
  NameDef(luaT_callTM)(L, tm, t1, t2, L->top, 1);  /* call TM */
  return !l_isfalse(L->top);
}


/* macro used by 'luaV_concat' to ensure that element at 'o' is a string */
#define tostring(L,o)  \
	(ttisstring(o) || (cvt2str(o) && (NameDef(luaO_tostring)(L, o), 1)))

#define isemptystr(o)	(ttisshrstring(o) && tsvalue(o)->shrlen == 0)

/* copy strings in stack from top - n up to top - 1 to buffer */
static void copy2buff (NameDef(StkId) top, int n, char *buff) {
  size_t tl = 0;  /* size already copied */
  do {
    size_t l = vslen(top - n);  /* length of string being copied */
    memcpy(buff + tl, svalue(top - n), l * sizeof(char));
    tl += l;
  } while (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top - total' up to 'L->top - 1'.
*/
void NameDef(luaV_concat) (NameDef(lua_State) *L, int total) {
  lua_assert(total >= 2);
  do {
    NameDef(StkId) top = L->top;
    int n = 2;  /* number of elements handled in this pass (at least 2) */
    if (!(ttisstring(top-2) || cvt2str(top-2)) || !tostring(L, top-1))
      NameDef(luaT_trybinTM)(L, top-2, top-1, top-2, NameDef(TM_CONCAT));
    else if (isemptystr(top - 1))  /* second operand is empty? */
      cast_void(tostring(L, top - 2));  /* result is first operand */
    else if (isemptystr(top - 2)) {  /* first operand is an empty string? */
      setobjs2s(L, top - 2, top - 1);  /* result is second op. */
    }
    else {
      /* at least two non-empty string values; get as many as possible */
      size_t tl = vslen(top - 1);
      NameDef(TString) *ts;
      /* collect total length and number of strings */
      for (n = 1; n < total && tostring(L, top - n - 1); n++) {
        size_t l = vslen(top - n - 1);
        if (l >= (MAX_SIZE/sizeof(char)) - tl)
          NameDef(luaG_runerror)(L, "string length overflow");
        tl += l;
      }
      if (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        char buff[LUAI_MAXSHORTLEN];
        copy2buff(top, n, buff);  /* copy strings to buffer */
        ts = NameDef(luaS_newlstr)(L, buff, tl);
      }
      else {  /* long string; copy strings directly to final result */
        ts = NameDef(luaS_createlngstrobj)(L, tl);
        copy2buff(top, n, getstr(ts));
      }
      setsvalue2s(L, top - n, ts);  /* create result */
    }
    total -= n-1;  /* got 'n' strings to create 1 new */
    L->top -= n-1;  /* popped 'n' strings and pushed one */
  } while (total > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra' = #rb'.
*/
void NameDef(luaV_objlen) (NameDef(lua_State) *L, NameDef(StkId) ra, const NameDef(TValue) *rb) {
  const NameDef(TValue) *tm;
  switch (ttype(rb)) {
    case LUA_TTABLE: {
      NameDef(Table) *h = hvalue(rb);
      tm = fasttm(L, h->metatable, NameDef(TM_LEN));
      if (tm) break;  /* metamethod? break switch to call it */
      setivalue(ra, NameDef(luaH_getn)(h));  /* else primitive len */
      return;
    }
    case LUA_TSHRSTR: {
      setivalue(ra, tsvalue(rb)->shrlen);
      return;
    }
    case LUA_TLNGSTR: {
      setivalue(ra, tsvalue(rb)->u.lnglen);
      return;
    }
    default: {  /* try metamethod */
      tm = NameDef(luaT_gettmbyobj)(L, rb, NameDef(TM_LEN));
      if (ttisnil(tm))  /* no metamethod? */
        NameDef(luaG_typeerror)(L, rb, "get length of");
      break;
    }
  }
  NameDef(luaT_callTM)(L, tm, rb, rb, ra, 1);
}


/*
** Integer division; return 'm // n', that is, floor(m/n).
** C division truncates its result (rounds towards zero).
** 'floor(q) == trunc(q)' when 'q >= 0' or when 'q' is integer,
** otherwise 'floor(q) == trunc(q) - 1'.
*/
NameDef(lua_Integer) NameDef(luaV_div) (NameDef(lua_State) *L, NameDef(lua_Integer) m, NameDef(lua_Integer) n) {
  if (l_castS2U(n) + 1u <= 1u) {  /* special cases: -1 or 0 */
    if (n == 0)
      NameDef(luaG_runerror)(L, "attempt to divide by zero");
    return intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  else {
    NameDef(lua_Integer) q = m / n;  /* perform C division */
    if ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    return q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_div.)
*/
NameDef(lua_Integer) NameDef(luaV_mod) (NameDef(lua_State) *L, NameDef(lua_Integer) m, NameDef(lua_Integer) n) {
  if (l_castS2U(n) + 1u <= 1u) {  /* special cases: -1 or 0 */
    if (n == 0)
      NameDef(luaG_runerror)(L, "attempt to perform 'n%%0'");
    return 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  else {
    NameDef(lua_Integer) r = m % n;
    if (r != 0 && (m ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    return r;
  }
}


/* number of bits in an integer */
#define NBITS	cast_int(sizeof(NameDef(lua_Integer)) * CHAR_BIT)

/*
** Shift left operation. (Shift right just negates 'y'.)
*/
NameDef(lua_Integer) NameDef(luaV_shiftl) (NameDef(lua_Integer) x, NameDef(lua_Integer) y) {
  if (y < 0) {  /* shift right? */
    if (y <= -NBITS) return 0;
    else return intop(>>, x, -y);
  }
  else {  /* shift left */
    if (y >= NBITS) return 0;
    else return intop(<<, x, y);
  }
}


/*
** check whether cached closure in prototype 'p' may be reused, that is,
** whether there is a cached closure with the same upvalues needed by
** new closure to be created.
*/
static NameDef(LClosure) *getcached (NameDef(Proto) *p, NameDef(UpVal) **encup, NameDef(StkId) base) {
  NameDef(LClosure) *c = p->cache;
  if (c != NULL) {  /* is there a cached closure? */
    int nup = p->sizeupvalues;
    NameDef(Upvaldesc) *uv = p->upvalues;
    int i;
    for (i = 0; i < nup; i++) {  /* check whether it has right upvalues */
      NameDef(TValue) *v = uv[i].instack ? base + uv[i].idx : encup[uv[i].idx]->v;
      if (c->upvals[i]->v != v)
        return NULL;  /* wrong upvalue; cannot reuse closure */
    }
  }
  return c;  /* return cached closure (or NULL if no cached closure) */
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues. Note that the closure is not cached if prototype is
** already black (which means that 'cache' was already cleared by the
** GC).
*/
static void pushclosure (NameDef(lua_State) *L, NameDef(Proto) *p, NameDef(UpVal) **encup, NameDef(StkId) base,
                         NameDef(StkId) ra) {
  int nup = p->sizeupvalues;
  NameDef(Upvaldesc) *uv = p->upvalues;
  int i;
  NameDef(LClosure) *ncl = NameDef(luaF_newLclosure)(L, nup);
  ncl->p = p;
  setclLvalue(L, ra, ncl);  /* anchor new closure in stack */
  for (i = 0; i < nup; i++) {  /* fill in its upvalues */
    if (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = NameDef(luaF_findupval)(L, base + uv[i].idx);
    else  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    ncl->upvals[i]->refcount++;
    /* new closure is white, so we do not need a barrier here */
  }
  if (!isblack(p))  /* cache will not break GC invariant? */
    p->cache = ncl;  /* save it on cache for reuse */
}


/*
** finish execution of an opcode interrupted by an yield
*/
void NameDef(luaV_finishOp) (NameDef(lua_State) *L) {
  NameDef(CallInfo) *ci = L->ci;
  NameDef(StkId) base = ci->u.l.base;
  NameDef(Instruction) inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  NameDef(OpCode) op = GET_OPCODE(inst);
  switch (op) {  /* finish its execution */
    case NameDef(OP_ADD): case NameDef(OP_SUB): case NameDef(OP_MUL): case NameDef(OP_DIV): case NameDef(OP_IDIV):
    case NameDef(OP_BAND): case NameDef(OP_BOR): case NameDef(OP_BXOR): case NameDef(OP_SHL): case NameDef(OP_SHR):
    case NameDef(OP_MOD): case NameDef(OP_POW):
    case NameDef(OP_UNM): case NameDef(OP_BNOT): case NameDef(OP_LEN):
    case NameDef(OP_GETTABUP): case NameDef(OP_GETTABLE): case NameDef(OP_SELF): {
      setobjs2s(L, base + GETARG_A(inst), --L->top);
      break;
    }
    case NameDef(OP_LE): case NameDef(OP_LT): case NameDef(OP_EQ): {
      int res = !l_isfalse(L->top - 1);
      L->top--;
      if (ci->callstatus & CIST_LEQ) {  /* "<=" using "<" instead? */
        lua_assert(op == OP_LE);
        ci->callstatus ^= CIST_LEQ;  /* clear mark */
        res = !res;  /* negate result */
      }
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      if (res != GETARG_A(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      break;
    }
    case NameDef(OP_CONCAT): {
      NameDef(StkId) top = L->top - 1;  /* top when 'luaT_trybinTM' was called */
      int b = GETARG_B(inst);      /* first element to concatenate */
      int total = cast_int(top - 1 - (base + b));  /* yet to concatenate */
      setobj2s(L, top - 2, top);  /* put TM result in proper position */
      if (total > 1) {  /* are there elements to concat? */
        L->top = top - 1;  /* top is one after last element (at top-2) */
        NameDef(luaV_concat)(L, total);  /* concat them (may yield again) */
      }
      /* move final result to final position */
      setobj2s(L, ci->u.l.base + GETARG_A(inst), L->top - 1);
      L->top = ci->top;  /* restore top */
      break;
    }
    case NameDef(OP_TFORCALL): {
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_TFORLOOP);
      L->top = ci->top;  /* correct top */
      break;
    }
    case NameDef(OP_CALL): {
      if (GETARG_C(inst) - 1 >= 0)  /* nresults >= 0? */
        L->top = ci->top;  /* adjust results */
      break;
    }
    case NameDef(OP_TAILCALL): case NameDef(OP_SETTABUP): case NameDef(OP_SETTABLE):
      break;
    default: lua_assert(0);
  }
}




/*
** {==================================================================
** Function 'luaV_execute': main interpreter loop
** ===================================================================
*/


/*
** some macros for common tasks in 'luaV_execute'
*/


#define RA(i)	(base+GETARG_A(i))
#define RB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgR, base+GETARG_B(i))
#define RC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgR, base+GETARG_C(i))
#define RKB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_B(i)) ? k+INDEXK(GETARG_B(i)) : base+GETARG_B(i))
#define RKC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_C(i)) ? k+INDEXK(GETARG_C(i)) : base+GETARG_C(i))


/* execute a jump instruction */
#define dojump(ci,i,e) \
  { int a = GETARG_A(i); \
    if (a != 0) NameDef(luaF_close)(L, ci->u.l.base + a - 1); \
    ci->u.l.savedpc += GETARG_sBx(i) + e; }

/* for test instructions, execute the jump instruction that follows it */
#define donextjump(ci)	{ i = *ci->u.l.savedpc; dojump(ci, i, 1); }


#define Protect(x)	{ {x;}; base = ci->u.l.base; }

#define checkGC(L,c)  \
	{ luaC_condGC(L, L->top = (c),  /* limit of live values */ \
                         Protect(L->top = ci->top));  /* restore top */ \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#define vmfetch()	{ \
  i = *(ci->u.l.savedpc++); \
  if (L->hookmask & (LUA_MASKLINE | LUA_MASKCOUNT)) \
    Protect(NameDef(luaG_traceexec)(L)); \
  ra = RA(i); /* WARNING: any stack reallocation invalidates 'ra' */ \
  lua_assert(base == ci->u.l.base); \
  lua_assert(base <= L->top && L->top < L->stack + L->stacksize); \
}

#define vmdispatch(o)	switch(o)
#define vmcase(l)	case l:
#define vmbreak		break


/*
** copy of 'luaV_gettable', but protecting the call to potential
** metamethod (which can reallocate the stack)
*/
#define gettableProtected(L,t,k,v)  { const NameDef(TValue) *slot; \
  if (luaV_fastget(L,t,k,slot,NameDef(luaH_get))) { setobj2s(L, v, slot); } \
  else Protect(NameDef(luaV_finishget)(L,t,k,v,slot)); }


/* same for 'luaV_settable' */
#define settableProtected(L,t,k,v) { const NameDef(TValue) *slot; \
  if (!luaV_fastset(L,t,k,slot,NameDef(luaH_get),v)) \
    Protect(NameDef(luaV_finishset)(L,t,k,v,slot)); }



void NameDef(luaV_execute) (NameDef(lua_State) *L) {
  NameDef(CallInfo) *ci = L->ci;
  NameDef(LClosure) *cl;
  NameDef(TValue) *k;
  NameDef(StkId) base;
  ci->callstatus |= CIST_FRESH;  /* fresh invocation of 'luaV_execute" */
 newframe:  /* reentry point when frame changes (call/return) */
  lua_assert(ci == L->ci);
  cl = clLvalue(ci->func);  /* local reference to function's closure */
  k = cl->p->k;  /* local reference to function's constant table */
  base = ci->u.l.base;  /* local copy of function's base */
  /* main loop of interpreter */
  for (;;) {
    NameDef(Instruction) i;
    NameDef(StkId) ra;
    vmfetch();
    vmdispatch (GET_OPCODE(i)) {
      vmcase(NameDef(OP_MOVE)) {
        setobjs2s(L, ra, RB(i));
        vmbreak;
      }
      vmcase(NameDef(OP_LOADK)) {
        NameDef(TValue) *rb = k + GETARG_Bx(i);
        setobj2s(L, ra, rb);
        vmbreak;
      }
      vmcase(NameDef(OP_LOADKX)) {
        NameDef(TValue) *rb;
        lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_EXTRAARG);
        rb = k + GETARG_Ax(*ci->u.l.savedpc++);
        setobj2s(L, ra, rb);
        vmbreak;
      }
      vmcase(NameDef(OP_LOADBOOL)) {
        setbvalue(ra, GETARG_B(i));
        if (GETARG_C(i)) ci->u.l.savedpc++;  /* skip next instruction (if C) */
        vmbreak;
      }
      vmcase(NameDef(OP_LOADNIL)) {
        int b = GETARG_B(i);
        do {
          setnilvalue(ra++);
        } while (b--);
        vmbreak;
      }
      vmcase(NameDef(OP_GETUPVAL)) {
        int b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v);
        vmbreak;
      }
      vmcase(NameDef(OP_GETTABUP)) {
        NameDef(TValue) *upval = cl->upvals[GETARG_B(i)]->v;
        NameDef(TValue) *rc = RKC(i);
        gettableProtected(L, upval, rc, ra);
        vmbreak;
      }
      vmcase(NameDef(OP_GETTABLE)) {
        NameDef(StkId) rb = RB(i);
        NameDef(TValue) *rc = RKC(i);
        gettableProtected(L, rb, rc, ra);
        vmbreak;
      }
      vmcase(NameDef(OP_SETTABUP)) {
        NameDef(TValue) *upval = cl->upvals[GETARG_A(i)]->v;
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        settableProtected(L, upval, rb, rc);
        vmbreak;
      }
      vmcase(NameDef(OP_SETUPVAL)) {
        NameDef(UpVal) *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v, ra);
        luaC_upvalbarrier(L, uv);
        vmbreak;
      }
      vmcase(NameDef(OP_SETTABLE)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        settableProtected(L, ra, rb, rc);
        vmbreak;
      }
      vmcase(NameDef(OP_NEWTABLE)) {
        int b = GETARG_B(i);
        int c = GETARG_C(i);
        NameDef(Table) *t = NameDef(luaH_new)(L);
        sethvalue(L, ra, t);
        if (b != 0 || c != 0)
          NameDef(luaH_resize)(L, t, NameDef(luaO_fb2int)(b), NameDef(luaO_fb2int)(c));
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(NameDef(OP_SELF)) {
        const NameDef(TValue) *aux;
        NameDef(StkId) rb = RB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(TString) *key = tsvalue(rc);  /* key must be a string */
        setobjs2s(L, ra + 1, rb);
        if (luaV_fastget(L, rb, key, aux, NameDef(luaH_getstr))) {
          setobj2s(L, ra, aux);
        }
        else Protect(NameDef(luaV_finishget)(L, rb, rc, ra, aux));
        vmbreak;
      }
      vmcase(NameDef(OP_ADD)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (ttisinteger(rb) && ttisinteger(rc)) {
          NameDef(lua_Integer) ib = ivalue(rb); NameDef(lua_Integer) ic = ivalue(rc);
          setivalue(ra, intop(+, ib, ic));
        }
        else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          setfltvalue(ra, luai_numadd(L, nb, nc));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_ADD))); }
        vmbreak;
      }
      vmcase(NameDef(OP_SUB)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (ttisinteger(rb) && ttisinteger(rc)) {
          NameDef(lua_Integer) ib = ivalue(rb); NameDef(lua_Integer) ic = ivalue(rc);
          setivalue(ra, intop(-, ib, ic));
        }
        else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          setfltvalue(ra, luai_numsub(L, nb, nc));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_SUB))); }
        vmbreak;
      }
      vmcase(NameDef(OP_MUL)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (ttisinteger(rb) && ttisinteger(rc)) {
          NameDef(lua_Integer) ib = ivalue(rb); NameDef(lua_Integer) ic = ivalue(rc);
          setivalue(ra, intop(*, ib, ic));
        }
        else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          setfltvalue(ra, luai_nummul(L, nb, nc));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_MUL))); }
        vmbreak;
      }
      vmcase(NameDef(OP_DIV)) {  /* float division (always with floats) */
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          setfltvalue(ra, luai_numdiv(L, nb, nc));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_DIV))); }
        vmbreak;
      }
      vmcase(NameDef(OP_BAND)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Integer) ib; NameDef(lua_Integer) ic;
        if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
          setivalue(ra, intop(&, ib, ic));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_BAND))); }
        vmbreak;
      }
      vmcase(NameDef(OP_BOR)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Integer) ib; NameDef(lua_Integer) ic;
        if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
          setivalue(ra, intop(|, ib, ic));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_BOR))); }
        vmbreak;
      }
      vmcase(NameDef(OP_BXOR)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Integer) ib; NameDef(lua_Integer) ic;
        if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
          setivalue(ra, intop(^, ib, ic));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_BXOR))); }
        vmbreak;
      }
      vmcase(NameDef(OP_SHL)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Integer) ib; NameDef(lua_Integer) ic;
        if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
          setivalue(ra, NameDef(luaV_shiftl)(ib, ic));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_SHL))); }
        vmbreak;
      }
      vmcase(NameDef(OP_SHR)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Integer) ib; NameDef(lua_Integer) ic;
        if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
          setivalue(ra, NameDef(luaV_shiftl)(ib, -ic));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_SHR))); }
        vmbreak;
      }
      vmcase(NameDef(OP_MOD)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (ttisinteger(rb) && ttisinteger(rc)) {
          NameDef(lua_Integer) ib = ivalue(rb); NameDef(lua_Integer) ic = ivalue(rc);
          setivalue(ra, NameDef(luaV_mod)(L, ib, ic));
        }
        else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          NameDef(lua_Number) m;
          luai_nummod(L, nb, nc, m);
          setfltvalue(ra, m);
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_MOD))); }
        vmbreak;
      }
      vmcase(NameDef(OP_IDIV)) {  /* floor division */
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (ttisinteger(rb) && ttisinteger(rc)) {
          NameDef(lua_Integer) ib = ivalue(rb); NameDef(lua_Integer) ic = ivalue(rc);
          setivalue(ra, NameDef(luaV_div)(L, ib, ic));
        }
        else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          setfltvalue(ra, luai_numidiv(L, nb, nc));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_IDIV))); }
        vmbreak;
      }
      vmcase(NameDef(OP_POW)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        NameDef(lua_Number) nb; NameDef(lua_Number) nc;
        if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
          setfltvalue(ra, luai_numpow(L, nb, nc));
        }
        else { Protect(NameDef(luaT_trybinTM)(L, rb, rc, ra, NameDef(TM_POW))); }
        vmbreak;
      }
      vmcase(NameDef(OP_UNM)) {
        NameDef(TValue) *rb = RB(i);
        NameDef(lua_Number) nb;
        if (ttisinteger(rb)) {
          NameDef(lua_Integer) ib = ivalue(rb);
          setivalue(ra, intop(-, 0, ib));
        }
        else if (tonumber(rb, &nb)) {
          setfltvalue(ra, luai_numunm(L, nb));
        }
        else {
          Protect(NameDef(luaT_trybinTM)(L, rb, rb, ra, NameDef(TM_UNM)));
        }
        vmbreak;
      }
      vmcase(NameDef(OP_BNOT)) {
        NameDef(TValue) *rb = RB(i);
        NameDef(lua_Integer) ib;
        if (tointeger(rb, &ib)) {
          setivalue(ra, intop(^, ~l_castS2U(0), ib));
        }
        else {
          Protect(NameDef(luaT_trybinTM)(L, rb, rb, ra, NameDef(TM_BNOT)));
        }
        vmbreak;
      }
      vmcase(NameDef(OP_NOT)) {
        NameDef(TValue) *rb = RB(i);
        int res = l_isfalse(rb);  /* next assignment may change this value */
        setbvalue(ra, res);
        vmbreak;
      }
      vmcase(NameDef(OP_LEN)) {
        Protect(NameDef(luaV_objlen)(L, ra, RB(i)));
        vmbreak;
      }
      vmcase(NameDef(OP_CONCAT)) {
        int b = GETARG_B(i);
        int c = GETARG_C(i);
        NameDef(StkId) rb;
        L->top = base + c + 1;  /* mark the end of concat operands */
        Protect(NameDef(luaV_concat)(L, c - b + 1));
        ra = RA(i);  /* 'luaV_concat' may invoke TMs and move the stack */
        rb = base + b;
        setobjs2s(L, ra, rb);
        checkGC(L, (ra >= rb ? ra + 1 : rb));
        L->top = ci->top;  /* restore top */
        vmbreak;
      }
      vmcase(NameDef(OP_JMP)) {
        dojump(ci, i, 0);
        vmbreak;
      }
      vmcase(NameDef(OP_EQ)) {
        NameDef(TValue) *rb = RKB(i);
        NameDef(TValue) *rc = RKC(i);
        Protect(
          if (NameDef(luaV_equalobj)(L, rb, rc) != GETARG_A(i))
            ci->u.l.savedpc++;
          else
            donextjump(ci);
        )
        vmbreak;
      }
      vmcase(NameDef(OP_LT)) {
        Protect(
          if (NameDef(luaV_lessthan)(L, RKB(i), RKC(i)) != GETARG_A(i))
            ci->u.l.savedpc++;
          else
            donextjump(ci);
        )
        vmbreak;
      }
      vmcase(NameDef(OP_LE)) {
        Protect(
          if (NameDef(luaV_lessequal)(L, RKB(i), RKC(i)) != GETARG_A(i))
            ci->u.l.savedpc++;
          else
            donextjump(ci);
        )
        vmbreak;
      }
      vmcase(NameDef(OP_TEST)) {
        if (GETARG_C(i) ? l_isfalse(ra) : !l_isfalse(ra))
            ci->u.l.savedpc++;
          else
          donextjump(ci);
        vmbreak;
      }
      vmcase(NameDef(OP_TESTSET)) {
        NameDef(TValue) *rb = RB(i);
        if (GETARG_C(i) ? l_isfalse(rb) : !l_isfalse(rb))
          ci->u.l.savedpc++;
        else {
          setobjs2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(NameDef(OP_CALL)) {
        int b = GETARG_B(i);
        int nresults = GETARG_C(i) - 1;
        if (b != 0) L->top = ra+b;  /* else previous instruction set top */
        if (NameDef(luaD_precall)(L, ra, nresults)) {  /* C function? */
          if (nresults >= 0)
            L->top = ci->top;  /* adjust results */
          Protect((void)0);  /* update 'base' */
        }
        else {  /* Lua function */
          ci = L->ci;
          goto newframe;  /* restart luaV_execute over new Lua function */
        }
        vmbreak;
      }
      vmcase(NameDef(OP_TAILCALL)) {
        int b = GETARG_B(i);
        if (b != 0) L->top = ra+b;  /* else previous instruction set top */
        lua_assert(GETARG_C(i) - 1 == LUA_MULTRET);
        if (NameDef(luaD_precall)(L, ra, LUA_MULTRET)) {  /* C function? */
          Protect((void)0);  /* update 'base' */
        }
        else {
          /* tail call: put called frame (n) in place of caller one (o) */
          NameDef(CallInfo) *nci = L->ci;  /* called frame */
          NameDef(CallInfo) *oci = nci->previous;  /* caller frame */
          NameDef(StkId) nfunc = nci->func;  /* called function */
          NameDef(StkId) ofunc = oci->func;  /* caller function */
          /* last stack slot filled by 'precall' */
          NameDef(StkId) lim = nci->u.l.base + getproto(nfunc)->numparams;
          int aux;
          /* close all upvalues from previous call */
          if (cl->p->sizep > 0) NameDef(luaF_close)(L, oci->u.l.base);
          /* move new frame into old one */
          for (aux = 0; nfunc + aux < lim; aux++)
            setobjs2s(L, ofunc + aux, nfunc + aux);
          oci->u.l.base = ofunc + (nci->u.l.base - nfunc);  /* correct base */
          oci->top = L->top = ofunc + (L->top - nfunc);  /* correct top */
          oci->u.l.savedpc = nci->u.l.savedpc;
          oci->callstatus |= CIST_TAIL;  /* function was tail called */
          ci = L->ci = oci;  /* remove new frame */
          lua_assert(L->top == oci->u.l.base + getproto(ofunc)->maxstacksize);
          goto newframe;  /* restart luaV_execute over new Lua function */
        }
        vmbreak;
      }
      vmcase(NameDef(OP_RETURN)) {
        int b = GETARG_B(i);
        if (cl->p->sizep > 0) NameDef(luaF_close)(L, base);
        b = NameDef(luaD_poscall)(L, ci, ra, (b != 0 ? b - 1 : cast_int(L->top - ra)));
        if (ci->callstatus & CIST_FRESH)  /* local 'ci' still from callee */
          return;  /* external invocation: return */
        else {  /* invocation via reentry: continue execution */
          ci = L->ci;
          if (b) L->top = ci->top;
          lua_assert(isLua(ci));
          lua_assert(GET_OPCODE(*((ci)->u.l.savedpc - 1)) == OP_CALL);
          goto newframe;  /* restart luaV_execute over new Lua function */
        }
      }
      vmcase(NameDef(OP_FORLOOP)) {
        if (ttisinteger(ra)) {  /* integer loop? */
          NameDef(lua_Integer) step = ivalue(ra + 2);
          NameDef(lua_Integer) idx = intop(+, ivalue(ra), step); /* increment index */
          NameDef(lua_Integer) limit = ivalue(ra + 1);
          if ((0 < step) ? (idx <= limit) : (limit <= idx)) {
            ci->u.l.savedpc += GETARG_sBx(i);  /* jump back */
            chgivalue(ra, idx);  /* update internal index... */
            setivalue(ra + 3, idx);  /* ...and external index */
          }
        }
        else {  /* floating loop */
          NameDef(lua_Number) step = fltvalue(ra + 2);
          NameDef(lua_Number) idx = luai_numadd(L, fltvalue(ra), step); /* inc. index */
          NameDef(lua_Number) limit = fltvalue(ra + 1);
          if (luai_numlt(0, step) ? luai_numle(idx, limit)
                                  : luai_numle(limit, idx)) {
            ci->u.l.savedpc += GETARG_sBx(i);  /* jump back */
            chgfltvalue(ra, idx);  /* update internal index... */
            setfltvalue(ra + 3, idx);  /* ...and external index */
          }
        }
        vmbreak;
      }
      vmcase(NameDef(OP_FORPREP)) {
        NameDef(TValue) *init = ra;
        NameDef(TValue) *plimit = ra + 1;
        NameDef(TValue) *pstep = ra + 2;
        NameDef(lua_Integer) ilimit;
        int stopnow;
        if (ttisinteger(init) && ttisinteger(pstep) &&
            forlimit(plimit, &ilimit, ivalue(pstep), &stopnow)) {
          /* all values are integer */
          NameDef(lua_Integer) initv = (stopnow ? 0 : ivalue(init));
          setivalue(plimit, ilimit);
          setivalue(init, intop(-, initv, ivalue(pstep)));
        }
        else {  /* try making all values floats */
          NameDef(lua_Number) ninit; NameDef(lua_Number) nlimit; NameDef(lua_Number) nstep;
          if (!tonumber(plimit, &nlimit))
            NameDef(luaG_runerror)(L, "'for' limit must be a number");
          setfltvalue(plimit, nlimit);
          if (!tonumber(pstep, &nstep))
            NameDef(luaG_runerror)(L, "'for' step must be a number");
          setfltvalue(pstep, nstep);
          if (!tonumber(init, &ninit))
            NameDef(luaG_runerror)(L, "'for' initial value must be a number");
          setfltvalue(init, luai_numsub(L, ninit, nstep));
        }
        ci->u.l.savedpc += GETARG_sBx(i);
        vmbreak;
      }
      vmcase(NameDef(OP_TFORCALL)) {
        NameDef(StkId) cb = ra + 3;  /* call base */
        setobjs2s(L, cb+2, ra+2);
        setobjs2s(L, cb+1, ra+1);
        setobjs2s(L, cb, ra);
        L->top = cb + 3;  /* func. + 2 args (state and index) */
        Protect(NameDef(luaD_call)(L, cb, GETARG_C(i)));
        L->top = ci->top;
        i = *(ci->u.l.savedpc++);  /* go to next instruction */
        ra = RA(i);
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP);
        goto l_tforloop;
      }
      vmcase(NameDef(OP_TFORLOOP)) {
        l_tforloop:
        if (!ttisnil(ra + 1)) {  /* continue loop? */
          setobjs2s(L, ra, ra + 1);  /* save control variable */
           ci->u.l.savedpc += GETARG_sBx(i);  /* jump back */
        }
        vmbreak;
      }
      vmcase(NameDef(OP_SETLIST)) {
        int n = GETARG_B(i);
        int c = GETARG_C(i);
        unsigned int last;
        NameDef(Table) *h;
        if (n == 0) n = cast_int(L->top - ra) - 1;
        if (c == 0) {
          lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_EXTRAARG);
          c = GETARG_Ax(*ci->u.l.savedpc++);
        }
        h = hvalue(ra);
        last = ((c-1)*LFIELDS_PER_FLUSH) + n;
        if (last > h->sizearray)  /* needs more space? */
          NameDef(luaH_resizearray)(L, h, last);  /* preallocate it at once */
        for (; n > 0; n--) {
          NameDef(TValue) *val = ra+n;
          NameDef(luaH_setint)(L, h, last--, val);
          luaC_barrierback(L, h, val);
        }
        L->top = ci->top;  /* correct top (in case of previous open call) */
        vmbreak;
      }
      vmcase(NameDef(OP_CLOSURE)) {
        NameDef(Proto) *p = cl->p->p[GETARG_Bx(i)];
        NameDef(LClosure) *ncl = getcached(p, cl->upvals, base);  /* cached closure */
        if (ncl == NULL)  /* no match? */
          pushclosure(L, p, cl->upvals, base, ra);  /* create a new one */
        else
          setclLvalue(L, ra, ncl);  /* push cashed closure */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(NameDef(OP_VARARG)) {
        int b = GETARG_B(i) - 1;  /* required results */
        int j;
        int n = cast_int(base - ci->func) - cl->p->numparams - 1;
        if (n < 0)  /* less arguments than parameters? */
          n = 0;  /* no vararg arguments */
        if (b < 0) {  /* B == 0? */
          b = n;  /* get all var. arguments */
          Protect(luaD_checkstack(L, n));
          ra = RA(i);  /* previous call may change the stack */
          L->top = ra + n;
        }
        for (j = 0; j < b && j < n; j++)
          setobjs2s(L, ra + j, base - n + j);
        for (; j < b; j++)  /* complete required results with nil */
          setnilvalue(ra + j);
        vmbreak;
      }
      vmcase(NameDef(OP_EXTRAARG)) {
        lua_assert(0);
        vmbreak;
      }
    }
  }
}

/* }================================================================== */

