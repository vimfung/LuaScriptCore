/*
** $Id: lbaselib.c,v 1.313 2016/04/11 19:18:40 roberto Exp $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static int luaB_print (NameDef(lua_State) *L) {
  int n = NameDef(lua_gettop)(L);  /* number of arguments */
  int i;
  NameDef(lua_getglobal)(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    size_t l;
    NameDef(lua_pushvalue)(L, -1);  /* function to be called */
    NameDef(lua_pushvalue)(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = NameDef(lua_tolstring)(L, -1, &l);  /* get result */
    if (s == NULL)
      return NameDef(luaL_error)(L, "'tostring' must return a string to 'print'");
    if (i>1) lua_writestring("\t", 1);
    lua_writestring(s, l);
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  return 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const char *b_str2int (const char *s, int base, NameDef(lua_Integer) *pn) {
  NameDef(lua_Unsigned) n = 0;
  int neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  if (*s == '-') { s++; neg = 1; }  /* handle signal */
  else if (*s == '+') s++;
  if (!isalnum((unsigned char)*s))  /* no digit? */
    return NULL;
  do {
    int digit = (isdigit((unsigned char)*s)) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (digit >= base) return NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } while (isalnum((unsigned char)*s));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (NameDef(lua_Integer))((neg) ? (0u - n) : n);
  return s;
}


static int luaB_tonumber (NameDef(lua_State) *L) {
  if (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    NameDef(luaL_checkany)(L, 1);
    if (NameDef(lua_type)(L, 1) == LUA_TNUMBER) {  /* already a number? */
      NameDef(lua_settop)(L, 1);  /* yes; return it */
      return 1;
    }
    else {
      size_t l;
      const char *s = NameDef(lua_tolstring)(L, 1, &l);
      if (s != NULL && NameDef(lua_stringtonumber)(L, s) == l + 1)
        return 1;  /* successful conversion to number */
      /* else not a number */
    }
  }
  else {
    size_t l;
    const char *s;
    NameDef(lua_Integer) n = 0;  /* to avoid warnings */
    NameDef(lua_Integer) base = NameDef(luaL_checkinteger)(L, 2);
    NameDef(luaL_checktype)(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = NameDef(lua_tolstring)(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    if (b_str2int(s, (int)base, &n) == s + l) {
      NameDef(lua_pushinteger)(L, n);
      return 1;
    }  /* else not a number */
  }  /* else not a number */
  NameDef(lua_pushnil)(L);  /* not a number */
  return 1;
}


static int luaB_error (NameDef(lua_State) *L) {
  int level = (int)NameDef(luaL_optinteger)(L, 2, 1);
  NameDef(lua_settop)(L, 1);
  if (NameDef(lua_type)(L, 1) == LUA_TSTRING && level > 0) {
    NameDef(luaL_where)(L, level);   /* add extra information */
    NameDef(lua_pushvalue)(L, 1);
    NameDef(lua_concat)(L, 2);
  }
  return NameDef(lua_error)(L);
}


static int luaB_getmetatable (NameDef(lua_State) *L) {
  NameDef(luaL_checkany)(L, 1);
  if (!NameDef(lua_getmetatable)(L, 1)) {
    NameDef(lua_pushnil)(L);
    return 1;  /* no metatable */
  }
  NameDef(luaL_getmetafield)(L, 1, "__metatable");
  return 1;  /* returns either __metatable field (if present) or metatable */
}


static int luaB_setmetatable (NameDef(lua_State) *L) {
  int t = NameDef(lua_type)(L, 2);
  NameDef(luaL_checktype)(L, 1, LUA_TTABLE);
  luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                    "nil or table expected");
  if (NameDef(luaL_getmetafield)(L, 1, "__metatable") != LUA_TNIL)
    return NameDef(luaL_error)(L, "cannot change a protected metatable");
  NameDef(lua_settop)(L, 2);
  NameDef(lua_setmetatable)(L, 1);
  return 1;
}


static int luaB_rawequal (NameDef(lua_State) *L) {
  NameDef(luaL_checkany)(L, 1);
  NameDef(luaL_checkany)(L, 2);
  NameDef(lua_pushboolean)(L, NameDef(lua_rawequal)(L, 1, 2));
  return 1;
}


static int luaB_rawlen (NameDef(lua_State) *L) {
  int t = NameDef(lua_type)(L, 1);
  luaL_argcheck(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                   "table or string expected");
  NameDef(lua_pushinteger)(L, NameDef(lua_rawlen)(L, 1));
  return 1;
}


static int luaB_rawget (NameDef(lua_State) *L) {
  NameDef(luaL_checktype)(L, 1, LUA_TTABLE);
  NameDef(luaL_checkany)(L, 2);
  NameDef(lua_settop)(L, 2);
  NameDef(lua_rawget)(L, 1);
  return 1;
}

static int luaB_rawset (NameDef(lua_State) *L) {
  NameDef(luaL_checktype)(L, 1, LUA_TTABLE);
  NameDef(luaL_checkany)(L, 2);
  NameDef(luaL_checkany)(L, 3);
  NameDef(lua_settop)(L, 3);
  NameDef(lua_rawset)(L, 1);
  return 1;
}


static int luaB_collectgarbage (NameDef(lua_State) *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", NULL};
  static const int optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCSETPAUSE, LUA_GCSETSTEPMUL,
    LUA_GCISRUNNING};
  int o = optsnum[NameDef(luaL_checkoption)(L, 1, "collect", opts)];
  int ex = (int)NameDef(luaL_optinteger)(L, 2, 0);
  int res = NameDef(lua_gc)(L, o, ex);
  switch (o) {
    case LUA_GCCOUNT: {
      int b = NameDef(lua_gc)(L, LUA_GCCOUNTB, 0);
      NameDef(lua_pushnumber)(L, (NameDef(lua_Number))res + ((NameDef(lua_Number))b/1024));
      return 1;
    }
    case LUA_GCSTEP: case LUA_GCISRUNNING: {
      NameDef(lua_pushboolean)(L, res);
      return 1;
    }
    default: {
      NameDef(lua_pushinteger)(L, res);
      return 1;
    }
  }
}


static int luaB_type (NameDef(lua_State) *L) {
  int t = NameDef(lua_type)(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  NameDef(lua_pushstring)(L, NameDef(lua_typename)(L, t));
  return 1;
}


static int pairsmeta (NameDef(lua_State) *L, const char *method, int iszero,
                      NameDef(lua_CFunction) iter) {
  if (NameDef(luaL_getmetafield)(L, 1, method) == LUA_TNIL) {  /* no metamethod? */
    NameDef(luaL_checktype)(L, 1, LUA_TTABLE);  /* argument must be a table */
    lua_pushcfunction(L, iter);  /* will return generator, */
    NameDef(lua_pushvalue)(L, 1);  /* state, */
    if (iszero) NameDef(lua_pushinteger)(L, 0);  /* and initial value */
    else NameDef(lua_pushnil)(L);
  }
  else {
    NameDef(lua_pushvalue)(L, 1);  /* argument 'self' to metamethod */
    lua_call(L, 1, 3);  /* get 3 values from metamethod */
  }
  return 3;
}


static int luaB_next (NameDef(lua_State) *L) {
  NameDef(luaL_checktype)(L, 1, LUA_TTABLE);
  NameDef(lua_settop)(L, 2);  /* create a 2nd argument if there isn't one */
  if (NameDef(lua_next)(L, 1))
    return 2;
  else {
    NameDef(lua_pushnil)(L);
    return 1;
  }
}


static int luaB_pairs (NameDef(lua_State) *L) {
  return pairsmeta(L, "__pairs", 0, luaB_next);
}


/*
** Traversal function for 'ipairs'
*/
static int ipairsaux (NameDef(lua_State) *L) {
  NameDef(lua_Integer) i = NameDef(luaL_checkinteger)(L, 2) + 1;
  NameDef(lua_pushinteger)(L, i);
  return (NameDef(lua_geti)(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static int luaB_ipairs (NameDef(lua_State) *L) {
#if defined(LUA_COMPAT_IPAIRS)
  return pairsmeta(L, "__ipairs", 1, ipairsaux);
#else
  NameDef(luaL_checkany)(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  NameDef(lua_pushvalue)(L, 1);  /* state */
  NameDef(lua_pushinteger)(L, 0);  /* initial value */
  return 3;
#endif
}


static int load_aux (NameDef(lua_State) *L, int status, int envidx) {
  if (status == LUA_OK) {
    if (envidx != 0) {  /* 'env' parameter? */
      NameDef(lua_pushvalue)(L, envidx);  /* environment for loaded function */
      if (!NameDef(lua_setupvalue)(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    NameDef(lua_pushnil)(L);
    lua_insert(L, -2);  /* put before error message */
    return 2;  /* return nil plus error message */
  }
}


static int luaB_loadfile (NameDef(lua_State) *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = NameDef(luaL_loadfilex)(L, fname, mode);
  return load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'lua_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader (NameDef(lua_State) *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  NameDef(luaL_checkstack)(L, 2, "too many nested functions");
  NameDef(lua_pushvalue)(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (!NameDef(lua_isstring)(L, -1))
    NameDef(luaL_error)(L, "reader function must return a string");
  lua_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
  return NameDef(lua_tolstring)(L, RESERVEDSLOT, size);
}


static int luaB_load (NameDef(lua_State) *L) {
  int status;
  size_t l;
  const char *s = NameDef(lua_tolstring)(L, 1, &l);
  const char *mode = luaL_optstring(L, 3, "bt");
  int env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = NameDef(luaL_loadbufferx)(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    NameDef(luaL_checktype)(L, 1, LUA_TFUNCTION);
    NameDef(lua_settop)(L, RESERVEDSLOT);  /* create reserved slot */
    status = NameDef(lua_load)(L, generic_reader, NULL, chunkname, mode);
  }
  return load_aux(L, status, env);
}

/* }====================================================== */


static int dofilecont (NameDef(lua_State) *L, int d1, NameDef(lua_KContext) d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  return NameDef(lua_gettop)(L) - 1;
}


static int luaB_dofile (NameDef(lua_State) *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  NameDef(lua_settop)(L, 1);
  if (luaL_loadfile(L, fname) != LUA_OK)
    return NameDef(lua_error)(L);
  NameDef(lua_callk)(L, 0, LUA_MULTRET, 0, dofilecont);
  return dofilecont(L, 0, 0);
}


static int luaB_assert (NameDef(lua_State) *L) {
  if (NameDef(lua_toboolean)(L, 1))  /* condition is true? */
    return NameDef(lua_gettop)(L);  /* return all arguments */
  else {  /* error */
    NameDef(luaL_checkany)(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    NameDef(lua_settop)(L, 1);  /* leave only message (default if no other one) */
    return luaB_error(L);  /* call 'error' */
  }
}


static int luaB_select (NameDef(lua_State) *L) {
  int n = NameDef(lua_gettop)(L);
  if (NameDef(lua_type)(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    NameDef(lua_pushinteger)(L, n-1);
    return 1;
  }
  else {
    NameDef(lua_Integer) i = NameDef(luaL_checkinteger)(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    return n - (int)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int finishpcall (NameDef(lua_State) *L, int status, NameDef(lua_KContext) extra) {
  if (status != LUA_OK && status != LUA_YIELD) {  /* error? */
    NameDef(lua_pushboolean)(L, 0);  /* first result (false) */
    NameDef(lua_pushvalue)(L, -2);  /* error message */
    return 2;  /* return false, msg */
  }
  else
    return NameDef(lua_gettop)(L) - (int)extra;  /* return all results */
}


static int luaB_pcall (NameDef(lua_State) *L) {
  int status;
  NameDef(luaL_checkany)(L, 1);
  NameDef(lua_pushboolean)(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = NameDef(lua_pcallk)(L, NameDef(lua_gettop)(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  return finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static int luaB_xpcall (NameDef(lua_State) *L) {
  int status;
  int n = NameDef(lua_gettop)(L);
  NameDef(luaL_checktype)(L, 2, LUA_TFUNCTION);  /* check error function */
  NameDef(lua_pushboolean)(L, 1);  /* first result */
  NameDef(lua_pushvalue)(L, 1);  /* function */
  NameDef(lua_rotate)(L, 3, 2);  /* move them below function's arguments */
  status = NameDef(lua_pcallk)(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  return finishpcall(L, status, 2);
}


static int luaB_tostring (NameDef(lua_State) *L) {
  NameDef(luaL_checkany)(L, 1);
  NameDef(luaL_tolstring)(L, 1, NULL);
  return 1;
}


static const NameDef(luaL_Reg) base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
#if defined(LUA_COMPAT_LOADSTRING)
  {"loadstring", luaB_load},
#endif
  {"next", luaB_next},
  {"pairs", luaB_pairs},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"xpcall", luaB_xpcall},
  /* placeholders */
  {"_G", NULL},
  {"_VERSION", NULL},
  {NULL, NULL}
};


LUAMOD_API int NameDef(luaopen_base) (NameDef(lua_State) *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  NameDef(luaL_setfuncs)(L, base_funcs, 0);
  /* set global _G */
  NameDef(lua_pushvalue)(L, -1);
  NameDef(lua_setfield)(L, -2, "_G");
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  NameDef(lua_setfield)(L, -2, "_VERSION");
  return 1;
}

