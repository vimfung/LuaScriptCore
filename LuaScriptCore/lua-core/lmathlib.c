/*
** $Id: lmathlib.c,v 1.115 2015/03/12 14:04:04 roberto Exp $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUA_LIB

#include "LuaDefine.h"

#include "lprefix.h"


#include <stdlib.h>
#include <math.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


#if !defined(l_rand)		/* { */
#if defined(LUA_USE_POSIX)
#define l_rand()	random()
#define l_srand(x)	srandom(x)
#define L_RANDMAX	2147483647	/* (2^31 - 1), following POSIX */
#else
#define l_rand()	rand()
#define l_srand(x)	srand(x)
#define L_RANDMAX	RAND_MAX
#endif
#endif				/* } */


static int math_abs (NameDef(lua_State) *L) {
  if (NameDef(lua_isinteger)(L, 1)) {
    NameDef(lua_Integer) n = lua_tointeger(L, 1);
    if (n < 0) n = (NameDef(lua_Integer))(0u - n);
    NameDef(lua_pushinteger)(L, n);
  }
  else
    NameDef(lua_pushnumber)(L, l_mathop(fabs)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_sin (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(sin)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_cos (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(cos)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_tan (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(tan)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_asin (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(asin)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_acos (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(acos)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_atan (NameDef(lua_State) *L) {
  NameDef(lua_Number) y = NameDef(luaL_checknumber)(L, 1);
  NameDef(lua_Number) x = NameDef(luaL_optnumber)(L, 2, 1);
  NameDef(lua_pushnumber)(L, l_mathop(atan2)(y, x));
  return 1;
}


static int math_toint (NameDef(lua_State) *L) {
  int valid;
  NameDef(lua_Integer) n = NameDef(lua_tointegerx)(L, 1, &valid);
  if (valid)
    NameDef(lua_pushinteger)(L, n);
  else {
    NameDef(luaL_checkany)(L, 1);
    NameDef(lua_pushnil)(L);  /* value is not convertible to integer */
  }
  return 1;
}


static void pushnumint (NameDef(lua_State) *L, NameDef(lua_Number) d) {
  NameDef(lua_Integer) n;
  if (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    NameDef(lua_pushinteger)(L, n);  /* result is integer */
  else
    NameDef(lua_pushnumber)(L, d);  /* result is float */
}


static int math_floor (NameDef(lua_State) *L) {
  if (NameDef(lua_isinteger)(L, 1))
    NameDef(lua_settop)(L, 1);  /* integer is its own floor */
  else {
    NameDef(lua_Number) d = l_mathop(floor)(NameDef(luaL_checknumber)(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_ceil (NameDef(lua_State) *L) {
  if (NameDef(lua_isinteger)(L, 1))
    NameDef(lua_settop)(L, 1);  /* integer is its own ceil */
  else {
    NameDef(lua_Number) d = l_mathop(ceil)(NameDef(luaL_checknumber)(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_fmod (NameDef(lua_State) *L) {
  if (NameDef(lua_isinteger)(L, 1) && NameDef(lua_isinteger)(L, 2)) {
    NameDef(lua_Integer) d = lua_tointeger(L, 2);
    if ((NameDef(lua_Unsigned))d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      NameDef(lua_pushinteger)(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      NameDef(lua_pushinteger)(L, lua_tointeger(L, 1) % d);
  }
  else
    NameDef(lua_pushnumber)(L, l_mathop(fmod)(NameDef(luaL_checknumber)(L, 1),
                                     NameDef(luaL_checknumber)(L, 2)));
  return 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
static int math_modf (NameDef(lua_State) *L) {
  if (NameDef(lua_isinteger)(L ,1)) {
    NameDef(lua_settop)(L, 1);  /* number is its own integer part */
    NameDef(lua_pushnumber)(L, 0);  /* no fractional part */
  }
  else {
    NameDef(lua_Number) n = NameDef(luaL_checknumber)(L, 1);
    /* integer part (rounds toward zero) */
    NameDef(lua_Number) ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    NameDef(lua_pushnumber)(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  return 2;
}


static int math_sqrt (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(sqrt)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}


static int math_ult (NameDef(lua_State) *L) {
  NameDef(lua_Integer) a = NameDef(luaL_checkinteger)(L, 1);
  NameDef(lua_Integer) b = NameDef(luaL_checkinteger)(L, 2);
  NameDef(lua_pushboolean)(L, (NameDef(lua_Unsigned))a < (NameDef(lua_Unsigned))b);
  return 1;
}

static int math_log (NameDef(lua_State) *L) {
  NameDef(lua_Number) x = NameDef(luaL_checknumber)(L, 1);
  NameDef(lua_Number) res;
  if (lua_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  else {
    NameDef(lua_Number) base = NameDef(luaL_checknumber)(L, 2);
#if !defined(LUA_USE_C89)
    if (base == 2.0) res = l_mathop(log2)(x); else
#endif
    if (base == 10.0) res = l_mathop(log10)(x);
    else res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  NameDef(lua_pushnumber)(L, res);
  return 1;
}

static int math_exp (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, l_mathop(exp)(NameDef(luaL_checknumber)(L, 1)));
  return 1;
}

static int math_deg (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, NameDef(luaL_checknumber)(L, 1) * (l_mathop(180.0) / PI));
  return 1;
}

static int math_rad (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, NameDef(luaL_checknumber)(L, 1) * (PI / l_mathop(180.0)));
  return 1;
}


static int math_min (NameDef(lua_State) *L) {
  int n = NameDef(lua_gettop)(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (NameDef(lua_compare)(L, i, imin, LUA_OPLT))
      imin = i;
  }
  NameDef(lua_pushvalue)(L, imin);
  return 1;
}


static int math_max (NameDef(lua_State) *L) {
  int n = NameDef(lua_gettop)(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (NameDef(lua_compare)(L, imax, i, LUA_OPLT))
      imax = i;
  }
  NameDef(lua_pushvalue)(L, imax);
  return 1;
}

/*
** This function uses 'double' (instead of 'lua_Number') to ensure that
** all bits from 'l_rand' can be represented, and that 'RANDMAX + 1.0'
** will keep full precision (ensuring that 'r' is always less than 1.0.)
*/
static int math_random (NameDef(lua_State) *L) {
  NameDef(lua_Integer) low, up;
  double r = (double)l_rand() * (1.0 / ((double)L_RANDMAX + 1.0));
  switch (NameDef(lua_gettop)(L)) {  /* check number of arguments */
    case 0: {  /* no arguments */
      NameDef(lua_pushnumber)(L, (NameDef(lua_Number))r);  /* Number between 0 and 1 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = NameDef(luaL_checkinteger)(L, 1);
      break;
    }
    case 2: {  /* lower and upper limits */
      low = NameDef(luaL_checkinteger)(L, 1);
      up = NameDef(luaL_checkinteger)(L, 2);
      break;
    }
    default: return NameDef(luaL_error)(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, low <= up, 1, "interval is empty"); 
  luaL_argcheck(L, low >= 0 || up <= LUA_MAXINTEGER + low, 1,
                   "interval too large");
  r *= (double)(up - low) + 1.0;
  NameDef(lua_pushinteger)(L, (NameDef(lua_Integer))r + low);
  return 1;
}


static int math_randomseed (NameDef(lua_State) *L) {
  l_srand((unsigned int)(NameDef(lua_Integer))NameDef(luaL_checknumber)(L, 1));
  (void)rand(); /* discard first value to avoid undesirable correlations */
  return 0;
}


static int math_type (NameDef(lua_State) *L) {
  if (NameDef(lua_type)(L, 1) == LUA_TNUMBER) {
      if (NameDef(lua_isinteger)(L, 1))
        lua_pushliteral(L, "integer"); 
      else
        lua_pushliteral(L, "float"); 
  }
  else {
    NameDef(luaL_checkany)(L, 1);
    NameDef(lua_pushnil)(L);
  }
  return 1;
}


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#if defined(LUA_COMPAT_MATHLIB)

static int math_cosh (NameDef(lua_State) *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sinh (NameDef(lua_State) *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tanh (NameDef(lua_State) *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_pow (NameDef(lua_State) *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_pushnumber(L, l_mathop(pow)(x, y));
  return 1;
}

static int math_frexp (NameDef(lua_State) *L) {
  int e;
  lua_pushnumber(L, l_mathop(frexp)(luaL_checknumber(L, 1), &e));
  lua_pushinteger(L, e);
  return 2;
}

static int math_ldexp (NameDef(lua_State) *L) {
  lua_Number x = luaL_checknumber(L, 1);
  int ep = (int)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  return 1;
}

static int math_log10 (NameDef(lua_State) *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  return 1;
}

#endif
/* }================================================================== */



static const NameDef(luaL_Reg) mathlib[] = {
  {"abs",   math_abs},
  {"acos",  math_acos},
  {"asin",  math_asin},
  {"atan",  math_atan},
  {"ceil",  math_ceil},
  {"cos",   math_cos},
  {"deg",   math_deg},
  {"exp",   math_exp},
  {"tointeger", math_toint},
  {"floor", math_floor},
  {"fmod",   math_fmod},
  {"ult",   math_ult},
  {"log",   math_log},
  {"max",   math_max},
  {"min",   math_min},
  {"modf",   math_modf},
  {"rad",   math_rad},
  {"random",     math_random},
  {"randomseed", math_randomseed},
  {"sin",   math_sin},
  {"sqrt",  math_sqrt},
  {"tan",   math_tan},
  {"type", math_type},
#if defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh",   math_cosh},
  {"sinh",   math_sinh},
  {"tanh",   math_tanh},
  {"pow",   math_pow},
  {"frexp", math_frexp},
  {"ldexp", math_ldexp},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"pi", NULL},
  {"huge", NULL},
  {"maxinteger", NULL},
  {"mininteger", NULL},
  {NULL, NULL}
};


/*
** Open math library
*/
LUAMOD_API int NameDef(luaopen_math) (NameDef(lua_State) *L) {
  luaL_newlib(L, mathlib);
  NameDef(lua_pushnumber)(L, PI);
  NameDef(lua_setfield)(L, -2, "pi");
  NameDef(lua_pushnumber)(L, (NameDef(lua_Number))HUGE_VAL);
  NameDef(lua_setfield)(L, -2, "huge");
  NameDef(lua_pushinteger)(L, LUA_MAXINTEGER);
  NameDef(lua_setfield)(L, -2, "maxinteger");
  NameDef(lua_pushinteger)(L, LUA_MININTEGER);
  NameDef(lua_setfield)(L, -2, "mininteger");
  return 1;
}

