/*
** $Id: lbitlib.c,v 1.30 2015/11/11 19:08:09 roberto Exp $
** Standard library for bitwise operations
** See Copyright Notice in lua.h
*/

#define lbitlib_c
#define LUA_LIB

#include "lprefix.h"


#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


#if defined(LUA_COMPAT_BITLIB)		/* { */


#define pushunsigned(L,n)	NameDef(lua_pushinteger)(L, (NameDef(lua_Integer))(n))
#define checkunsigned(L,i)	((NameDef(lua_Unsigned))luaL_checkinteger(L,i))


/* number of bits to consider in a number */
#if !defined(LUA_NBITS)
#define LUA_NBITS	32
#endif


/*
** a lua_Unsigned with its first LUA_NBITS bits equal to 1. (Shift must
** be made in two parts to avoid problems when LUA_NBITS is equal to the
** number of bits in a lua_Unsigned.)
*/
#define ALLONES		(~(((~(NameDef(lua_Unsigned))0) << (LUA_NBITS - 1)) << 1))


/* macro to trim extra bits */
#define trim(x)		((x) & ALLONES)


/* builds a number with 'n' ones (1 <= n <= LUA_NBITS) */
#define mask(n)		(~((ALLONES << 1) << ((n) - 1)))



static NameDef(lua_Unsigned) andaux (NameDef(lua_State) *L) {
  int i, n = NameDef(lua_gettop)(L);
  NameDef(lua_Unsigned) r = ~(NameDef(lua_Unsigned))0;
  for (i = 1; i <= n; i++)
    r &= checkunsigned(L, i);
  return trim(r);
}


static int b_and (NameDef(lua_State) *L) {
  NameDef(lua_Unsigned) r = andaux(L);
  pushunsigned(L, r);
  return 1;
}


static int b_test (NameDef(lua_State) *L) {
  NameDef(lua_Unsigned) r = andaux(L);
  NameDef(lua_pushboolean)(L, r != 0);
  return 1;
}


static int b_or (NameDef(lua_State) *L) {
  int i, n = NameDef(lua_gettop)(L);
  NameDef(lua_Unsigned) r = 0;
  for (i = 1; i <= n; i++)
    r |= checkunsigned(L, i);
  pushunsigned(L, trim(r));
  return 1;
}


static int b_xor (NameDef(lua_State) *L) {
  int i, n = NameDef(lua_gettop)(L);
  NameDef(lua_Unsigned) r = 0;
  for (i = 1; i <= n; i++)
    r ^= checkunsigned(L, i);
  pushunsigned(L, trim(r));
  return 1;
}


static int b_not (NameDef(lua_State) *L) {
  NameDef(lua_Unsigned) r = ~checkunsigned(L, 1);
  pushunsigned(L, trim(r));
  return 1;
}


static int b_shift (NameDef(lua_State) *L, NameDef(lua_Unsigned) r, NameDef(lua_Integer) i) {
  if (i < 0) {  /* shift right? */
    i = -i;
    r = trim(r);
    if (i >= LUA_NBITS) r = 0;
    else r >>= i;
  }
  else {  /* shift left */
    if (i >= LUA_NBITS) r = 0;
    else r <<= i;
    r = trim(r);
  }
  pushunsigned(L, r);
  return 1;
}


static int b_lshift (NameDef(lua_State) *L) {
  return b_shift(L, checkunsigned(L, 1), luaL_checkinteger(L, 2));
}


static int b_rshift (NameDef(lua_State) *L) {
  return b_shift(L, checkunsigned(L, 1), -luaL_checkinteger(L, 2));
}


static int b_arshift (NameDef(lua_State) *L) {
  NameDef(lua_Unsigned) r = checkunsigned(L, 1);
  NameDef(lua_Integer) i = luaL_checkinteger(L, 2);
  if (i < 0 || !(r & ((NameDef(lua_Unsigned))1 << (LUA_NBITS - 1))))
    return b_shift(L, r, -i);
  else {  /* arithmetic shift for 'negative' number */
    if (i >= LUA_NBITS) r = ALLONES;
    else
      r = trim((r >> i) | ~(trim(~(NameDef(lua_Unsigned))0) >> i));  /* add signal bit */
    pushunsigned(L, r);
    return 1;
  }
}


static int b_rot (NameDef(lua_State) *L, NameDef(lua_Integer) d) {
  NameDef(lua_Unsigned) r = checkunsigned(L, 1);
  int i = d & (LUA_NBITS - 1);  /* i = d % NBITS */
  r = trim(r);
  if (i != 0)  /* avoid undefined shift of LUA_NBITS when i == 0 */
    r = (r << i) | (r >> (LUA_NBITS - i));
  pushunsigned(L, trim(r));
  return 1;
}


static int b_lrot (NameDef(lua_State) *L) {
  return b_rot(L, luaL_checkinteger(L, 2));
}


static int b_rrot (NameDef(lua_State) *L) {
  return b_rot(L, -luaL_checkinteger(L, 2));
}


/*
** get field and width arguments for field-manipulation functions,
** checking whether they are valid.
** ('luaL_error' called without 'return' to avoid later warnings about
** 'width' being used uninitialized.)
*/
static int fieldargs (NameDef(lua_State) *L, int farg, int *width) {
  NameDef(lua_Integer) f = luaL_checkinteger(L, farg);
  NameDef(lua_Integer) w = luaL_optinteger(L, farg + 1, 1);
  luaL_argcheck(L, 0 <= f, farg, "field cannot be negative");
  luaL_argcheck(L, 0 < w, farg + 1, "width must be positive");
  if (f + w > LUA_NBITS)
    luaL_error(L, "trying to access non-existent bits");
  *width = (int)w;
  return (int)f;
}


static int b_extract (NameDef(lua_State) *L) {
  int w;
  NameDef(lua_Unsigned) r = trim(checkunsigned(L, 1));
  int f = fieldargs(L, 2, &w);
  r = (r >> f) & mask(w);
  pushunsigned(L, r);
  return 1;
}


static int b_replace (NameDef(lua_State) *L) {
  int w;
  NameDef(lua_Unsigned) r = trim(checkunsigned(L, 1));
  NameDef(lua_Unsigned) v = trim(checkunsigned(L, 2));
  int f = fieldargs(L, 3, &w);
  NameDef(lua_Unsigned) m = mask(w);
  r = (r & ~(m << f)) | ((v & m) << f);
  pushunsigned(L, r);
  return 1;
}


static const luaL_Reg bitlib[] = {
  {"arshift", b_arshift},
  {"band", b_and},
  {"bnot", b_not},
  {"bor", b_or},
  {"bxor", b_xor},
  {"btest", b_test},
  {"extract", b_extract},
  {"lrotate", b_lrot},
  {"lshift", b_lshift},
  {"replace", b_replace},
  {"rrotate", b_rrot},
  {"rshift", b_rshift},
  {NULL, NULL}
};



LUAMOD_API int NameDef(luaopen_bit32) (NameDef(lua_State) *L) {
  luaL_newlib(L, bitlib);
  return 1;
}


#else					/* }{ */


LUAMOD_API int NameDef(luaopen_bit32) (NameDef(lua_State) *L) {
  return NameDef(luaL_error)(L, "library 'bit32' has been deprecated");
}

#endif					/* } */
