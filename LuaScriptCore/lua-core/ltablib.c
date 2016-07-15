/*
** $Id: ltablib.c,v 1.80 2015/01/13 16:27:29 roberto Exp $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#define ltablib_c
#define LUA_LIB

#include "LuaDefine.h"

#include "lprefix.h"


#include <limits.h>
#include <stddef.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"



/*
** Structure with table-access functions
*/
typedef struct {
  int (*geti) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n);
  void (*seti) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n);
} NameDef(TabA);


/*
** Check that 'arg' has a table and set access functions in 'ta' to raw
** or non-raw according to the presence of corresponding metamethods.
*/
static void checktab (NameDef(lua_State) *L, int arg, NameDef(TabA) *ta) {
  ta->geti = NULL; ta->seti = NULL;
  if (NameDef(lua_getmetatable)(L, arg)) {
    lua_pushliteral(L, "__index");  /* 'index' metamethod */
    if (NameDef(lua_rawget)(L, -2) != LUA_TNIL)
      ta->geti = NameDef(lua_geti);
    lua_pushliteral(L, "__newindex");  /* 'newindex' metamethod */
    if (NameDef(lua_rawget)(L, -3) != LUA_TNIL)
      ta->seti = NameDef(lua_seti);
    lua_pop(L, 3);  /* pop metatable plus both metamethods */
  }
  if (ta->geti == NULL || ta->seti == NULL) {
    NameDef(luaL_checktype)(L, arg, LUA_TTABLE);  /* must be table for raw methods */
    if (ta->geti == NULL) ta->geti = NameDef(lua_rawgeti);
    if (ta->seti == NULL) ta->seti = NameDef(lua_rawseti);
  }
}


#define aux_getn(L,n,ta)	(checktab(L, n, ta), NameDef(luaL_len)(L, n))


#if defined(LUA_COMPAT_MAXN)
static int maxn (lua_State *L) {
  lua_Number max = 0;
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushnil(L);  /* first key */
  while (lua_next(L, 1)) {
    lua_pop(L, 1);  /* remove value */
    if (lua_type(L, -1) == LUA_TNUMBER) {
      lua_Number v = lua_tonumber(L, -1);
      if (v > max) max = v;
    }
  }
  lua_pushnumber(L, max);
  return 1;
}
#endif


static int tinsert (NameDef(lua_State) *L) {
  NameDef(TabA) ta;
  NameDef(lua_Integer) e = aux_getn(L, 1, &ta) + 1;  /* first empty element */
  NameDef(lua_Integer) pos;  /* where to insert new element */
  switch (NameDef(lua_gettop)(L)) {
    case 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      break;
    }
    case 3: {
      NameDef(lua_Integer) i;
      pos = NameDef(luaL_checkinteger)(L, 2);  /* 2nd argument is the position */
      luaL_argcheck(L, 1 <= pos && pos <= e, 2, "position out of bounds");
      for (i = e; i > pos; i--) {  /* move up elements */
        (*ta.geti)(L, 1, i - 1);
        (*ta.seti)(L, 1, i);  /* t[i] = t[i - 1] */
      }
      break;
    }
    default: {
      return NameDef(luaL_error)(L, "wrong number of arguments to 'insert'");
    }
  }
  (*ta.seti)(L, 1, pos);  /* t[pos] = v */
  return 0;
}


static int tremove (NameDef(lua_State) *L) {
  NameDef(TabA) ta;
  NameDef(lua_Integer) size = aux_getn(L, 1, &ta);
  NameDef(lua_Integer) pos = NameDef(luaL_optinteger)(L, 2, size);
  if (pos != size)  /* validate 'pos' if given */
    luaL_argcheck(L, 1 <= pos && pos <= size + 1, 1, "position out of bounds");
  (*ta.geti)(L, 1, pos);  /* result = t[pos] */
  for ( ; pos < size; pos++) {
    (*ta.geti)(L, 1, pos + 1);
    (*ta.seti)(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  NameDef(lua_pushnil)(L);
  (*ta.seti)(L, 1, pos);  /* t[pos] = nil */
  return 1;
}


static int tmove (NameDef(lua_State) *L) {
  NameDef(TabA) ta;
  NameDef(lua_Integer) f = NameDef(luaL_checkinteger)(L, 2);
  NameDef(lua_Integer) e = NameDef(luaL_checkinteger)(L, 3);
  NameDef(lua_Integer) t = NameDef(luaL_checkinteger)(L, 4);
  int tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  if (e >= f) {  /* otherwise, nothing to move */
    NameDef(lua_Integer) n, i;
    ta.geti = (NameDef(luaL_getmetafield)(L, 1, "__index") == LUA_TNIL)
      ? (NameDef(luaL_checktype)(L, 1, LUA_TTABLE), NameDef(lua_rawgeti))
      : NameDef(lua_geti);
    ta.seti = (NameDef(luaL_getmetafield)(L, tt, "__newindex") == LUA_TNIL)
      ? (NameDef(luaL_checktype)(L, tt, LUA_TTABLE), NameDef(lua_rawseti))
      : NameDef(lua_seti);
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    if (t > f) {
      for (i = n - 1; i >= 0; i--) {
        (*ta.geti)(L, 1, f + i);
        (*ta.seti)(L, tt, t + i);
      }
    }
    else {
      for (i = 0; i < n; i++) {
        (*ta.geti)(L, 1, f + i);
        (*ta.seti)(L, tt, t + i);
      }
    }
  }
  NameDef(lua_pushvalue)(L, tt);  /* return "to table" */
  return 1;
}


static void addfield (NameDef(lua_State) *L, NameDef(luaL_Buffer) *b, NameDef(TabA) *ta, NameDef(lua_Integer) i) {
  (*ta->geti)(L, 1, i);
  if (!NameDef(lua_isstring)(L, -1))
    NameDef(luaL_error)(L, "invalid value (%s) at index %d in table for 'concat'",
                  luaL_typename(L, -1), i);
  NameDef(luaL_addvalue)(b);
}


static int tconcat (NameDef(lua_State) *L) {
  NameDef(TabA) ta;
  NameDef(luaL_Buffer) b;
  size_t lsep;
  NameDef(lua_Integer) i, last;
  const char *sep = NameDef(luaL_optlstring)(L, 2, "", &lsep);
  checktab(L, 1, &ta);
  i = NameDef(luaL_optinteger)(L, 3, 1);
  last = luaL_opt(L, NameDef(luaL_checkinteger), 4, NameDef(luaL_len)(L, 1));
  NameDef(luaL_buffinit)(L, &b);
  for (; i < last; i++) {
    addfield(L, &b, &ta, i);
    NameDef(luaL_addlstring)(&b, sep, lsep);
  }
  if (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, &ta, i);
  NameDef(luaL_pushresult)(&b);
  return 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

static int pack (NameDef(lua_State) *L) {
  int i;
  int n = NameDef(lua_gettop)(L);  /* number of elements to pack */
  NameDef(lua_createtable)(L, n, 1);  /* create result table */
  lua_insert(L, 1);  /* put it at index 1 */
  for (i = n; i >= 1; i--)  /* assign elements */
    NameDef(lua_rawseti)(L, 1, i);
  NameDef(lua_pushinteger)(L, n);
  NameDef(lua_setfield)(L, 1, "n");  /* t.n = number of elements */
  return 1;  /* return table */
}


static int unpack (NameDef(lua_State) *L) {
  NameDef(TabA) ta;
  NameDef(lua_Integer) i, e;
  NameDef(lua_Unsigned) n;
  checktab(L, 1, &ta);
  i = NameDef(luaL_optinteger)(L, 2, 1);
  e = luaL_opt(L, NameDef(luaL_checkinteger), 3, NameDef(luaL_len)(L, 1));
  if (i > e) return 0;  /* empty range */
  n = (NameDef(lua_Unsigned))e - i;  /* number of elements minus 1 (avoid overflows) */
  if (n >= (unsigned int)INT_MAX  || !NameDef(lua_checkstack)(L, (int)(++n)))
    return NameDef(luaL_error)(L, "too many results to unpack");
  do {  /* must have at least one element */
    (*ta.geti)(L, 1, i);  /* push arg[i..e] */
  } while (i++ < e); 

  return (int)n;
}

/* }====================================================== */



/*
** {======================================================
** Quicksort
** (based on 'Algorithms in MODULA-3', Robert Sedgewick;
**  Addison-Wesley, 1993.)
** =======================================================
*/


static void set2 (NameDef(lua_State) *L, NameDef(TabA) *ta, int i, int j) {
  (*ta->seti)(L, 1, i);
  (*ta->seti)(L, 1, j);
}

static int sort_comp (NameDef(lua_State) *L, int a, int b) {
  if (!lua_isnil(L, 2)) {  /* function? */
    int res;
    NameDef(lua_pushvalue)(L, 2);
    NameDef(lua_pushvalue)(L, a-1);  /* -1 to compensate function */
    NameDef(lua_pushvalue)(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);
    res = NameDef(lua_toboolean)(L, -1);
    lua_pop(L, 1);
    return res;
  }
  else  /* a < b? */
    return NameDef(lua_compare)(L, a, b, LUA_OPLT);
}

static void auxsort (NameDef(lua_State) *L, NameDef(TabA) *ta, int l, int u) {
  while (l < u) {  /* for tail recursion */
    int i, j;
    /* sort elements a[l], a[(l+u)/2] and a[u] */
    (*ta->geti)(L, 1, l);
    (*ta->geti)(L, 1, u);
    if (sort_comp(L, -1, -2))  /* a[u] < a[l]? */
      set2(L, ta, l, u);  /* swap a[l] - a[u] */
    else
      lua_pop(L, 2);
    if (u-l == 1) break;  /* only 2 elements */
    i = (l+u)/2;
    (*ta->geti)(L, 1, i);
    (*ta->geti)(L, 1, l);
    if (sort_comp(L, -2, -1))  /* a[i]<a[l]? */
      set2(L, ta, i, l);
    else {
      lua_pop(L, 1);  /* remove a[l] */
      (*ta->geti)(L, 1, u);
      if (sort_comp(L, -1, -2))  /* a[u]<a[i]? */
        set2(L, ta, i, u);
      else
        lua_pop(L, 2);
    }
    if (u-l == 2) break;  /* only 3 elements */
    (*ta->geti)(L, 1, i);  /* Pivot */
    NameDef(lua_pushvalue)(L, -1);
    (*ta->geti)(L, 1, u-1);
    set2(L, ta, i, u-1);
    /* a[l] <= P == a[u-1] <= a[u], only need to sort from l+1 to u-2 */
    i = l; j = u-1;
    for (;;) {  /* invariant: a[l..i] <= P <= a[j..u] */
      /* repeat ++i until a[i] >= P */
      while ((*ta->geti)(L, 1, ++i), sort_comp(L, -1, -2)) {
        if (i>=u) NameDef(luaL_error)(L, "invalid order function for sorting");
        lua_pop(L, 1);  /* remove a[i] */
      }
      /* repeat --j until a[j] <= P */
      while ((*ta->geti)(L, 1, --j), sort_comp(L, -3, -1)) {
        if (j<=l) NameDef(luaL_error)(L, "invalid order function for sorting");
        lua_pop(L, 1);  /* remove a[j] */
      }
      if (j<i) {
        lua_pop(L, 3);  /* pop pivot, a[i], a[j] */
        break;
      }
      set2(L, ta, i, j);
    }
    (*ta->geti)(L, 1, u-1);
    (*ta->geti)(L, 1, i);
    set2(L, ta, u-1, i);  /* swap pivot (a[u-1]) with a[i] */
    /* a[l..i-1] <= a[i] == P <= a[i+1..u] */
    /* adjust so that smaller half is in [j..i] and larger one in [l..u] */
    if (i-l < u-i) {
      j=l; i=i-1; l=i+2;
    }
    else {
      j=i+1; i=u; u=j-2;
    }
    auxsort(L, ta, j, i);  /* call recursively the smaller one */
  }  /* repeat the routine for the larger one */
}

static int sort (NameDef(lua_State) *L) {
  NameDef(TabA) ta;
  int n = (int)aux_getn(L, 1, &ta);
  NameDef(luaL_checkstack)(L, 50, "");  /* assume array is smaller than 2^50 */
  if (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
    NameDef(luaL_checktype)(L, 2, LUA_TFUNCTION);
  NameDef(lua_settop)(L, 2);  /* make sure there are two arguments */
  auxsort(L, &ta, 1, n);
  return 0;
}

/* }====================================================== */


static const NameDef(luaL_Reg) tab_funcs[] = {
  {"concat", tconcat},
#if defined(LUA_COMPAT_MAXN)
  {"maxn", maxn},
#endif
  {"insert", tinsert},
  {"pack", pack},
  {"unpack", unpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {NULL, NULL}
};


LUAMOD_API int NameDef(luaopen_table) (NameDef(lua_State) *L) {
  luaL_newlib(L, tab_funcs);
#if defined(LUA_COMPAT_UNPACK)
  /* _G.unpack = table.unpack */
  lua_getfield(L, -1, "unpack");
  lua_setglobal(L, "unpack");
#endif
  return 1;
}

