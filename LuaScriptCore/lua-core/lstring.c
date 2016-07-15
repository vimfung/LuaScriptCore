/*
** $Id: lstring.c,v 2.49 2015/06/01 16:34:37 roberto Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#include "LuaDefine.h"

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"


#define MEMERRMSG       "not enough memory"


/*
** Lua will use at most ~(2^LUAI_HASHLIMIT) bytes from a string to
** compute its hash
*/
#if !defined(LUAI_HASHLIMIT)
#define LUAI_HASHLIMIT		5
#endif


/*
** equality for long strings
*/
int NameDef(luaS_eqlngstr) (NameDef(TString) *a, NameDef(TString) *b) {
  size_t len = a->u.lnglen;
  lua_assert(a->tt == LUA_TLNGSTR && b->tt == LUA_TLNGSTR);
  return (a == b) ||  /* same instance or... */
    ((len == b->u.lnglen) &&  /* equal length and ... */
     (memcmp(getstr(a), getstr(b), len) == 0));  /* equal contents */
}


unsigned int  NameDef(luaS_hash) (const char *str, size_t l, unsigned int seed) {
  unsigned int h = seed ^ cast(unsigned int, l);
  size_t l1;
  size_t step = (l >> LUAI_HASHLIMIT) + 1;
  for (l1 = l; l1 >= step; l1 -= step)
    h = h ^ ((h<<5) + (h>>2) + cast_byte(str[l1 - 1]));
  return h;
}


/*
** resizes the string table
*/
void  NameDef(luaS_resize) (NameDef(lua_State) *L, int newsize) {
  int i;
  NameDef(stringtable) *tb = &G(L)->strt;
  if (newsize > tb->size) {  /* grow table if needed */
    luaM_reallocvector(L, tb->hash, tb->size, newsize, NameDef(TString) *);
    for (i = tb->size; i < newsize; i++)
      tb->hash[i] = NULL;
  }
  for (i = 0; i < tb->size; i++) {  /* rehash */
    NameDef(TString) *p = tb->hash[i];
    tb->hash[i] = NULL;
    while (p) {  /* for each node in the list */
      NameDef(TString) *hnext = p->u.hnext;  /* save next */
      unsigned int h = lmod(p->hash, newsize);  /* new position */
      p->u.hnext = tb->hash[h];  /* chain it */
      tb->hash[h] = p;
      p = hnext;
    }
  }
  if (newsize < tb->size) {  /* shrink table if needed */
    /* vanishing slice should be empty */
    lua_assert(tb->hash[newsize] == NULL && tb->hash[tb->size - 1] == NULL);
    luaM_reallocvector(L, tb->hash, tb->size, newsize, NameDef(TString) *);
  }
  tb->size = newsize;
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
void  NameDef(luaS_clearcache) (NameDef(global_State) *g) {
  int i;
  for (i = 0; i < STRCACHE_SIZE; i++) {
    if (iswhite(g->strcache[i][0]))  /* will entry be collected? */
      g->strcache[i][0] = g->memerrmsg;  /* replace it with something fixed */
  }
}


/*
** Initialize the string table and the string cache
*/
void  NameDef(luaS_init) (NameDef(lua_State) *L) {
  NameDef(global_State) *g = G(L);
  int i;
  NameDef(luaS_resize)(L, MINSTRTABSIZE);  /* initial size of string table */
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  NameDef(luaC_fix)(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  for (i = 0; i < STRCACHE_SIZE; i++)  /* fill cache with valid strings */
    g->strcache[i][0] = g->memerrmsg;
}



/*
** creates a new string object
*/
static NameDef(TString) *createstrobj (NameDef(lua_State) *L, const char *str, size_t l,
                              int tag, unsigned int h) {
  NameDef(TString) *ts;
  NameDef(GCObject) *o;
  size_t totalsize;  /* total size of TString object */
  totalsize = sizelstring(l);
  o = NameDef(luaC_newobj)(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  memcpy(getaddrstr(ts), str, l * sizeof(char));
  getaddrstr(ts)[l] = '\0';  /* ending 0 */
  return ts;
}


void  NameDef(luaS_remove) (NameDef(lua_State) *L, NameDef(TString) *ts) {
  NameDef(stringtable) *tb = &G(L)->strt;
  NameDef(TString) **p = &tb->hash[lmod(ts->hash, tb->size)];
  while (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


/*
** checks whether short string exists and reuses it or creates a new one
*/
static NameDef(TString) *internshrstr (NameDef(lua_State) *L, const char *str, size_t l) {
  NameDef(TString) *ts;
  NameDef(global_State) *g = G(L);
  unsigned int h = NameDef(luaS_hash)(str, l, g->seed);
  NameDef(TString) **list = &g->strt.hash[lmod(h, g->strt.size)];
  for (ts = *list; ts != NULL; ts = ts->u.hnext) {
    if (l == ts->shrlen &&
        (memcmp(str, getstr(ts), l * sizeof(char)) == 0)) {
      /* found! */
      if (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      return ts;
    }
  }
  if (g->strt.nuse >= g->strt.size && g->strt.size <= MAX_INT/2) {
    NameDef(luaS_resize)(L, g->strt.size * 2);
    list = &g->strt.hash[lmod(h, g->strt.size)];  /* recompute with new size */
  }
  ts = createstrobj(L, str, l, LUA_TSHRSTR, h);
  ts->shrlen = cast_byte(l);
  ts->u.hnext = *list;
  *list = ts;
  g->strt.nuse++;
  return ts;
}


/*
** new string (with explicit length)
*/
NameDef(TString) * NameDef(luaS_newlstr) (NameDef(lua_State) *L, const char *str, size_t l) {
  if (l <= LUAI_MAXSHORTLEN)  /* short string? */
    return internshrstr(L, str, l);
  else {
    NameDef(TString) *ts;
    if (l + 1 > (MAX_SIZE - sizeof(NameDef(TString)))/sizeof(char))
      NameDef(luaM_toobig)(L);
    ts = createstrobj(L, str, l, LUA_TLNGSTR, G(L)->seed);
    ts->u.lnglen = l;
    return ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
NameDef(TString) * NameDef(luaS_new) (NameDef(lua_State) *L, const char *str) {
  unsigned int i = point2uint(str) % STRCACHE_SIZE;  /* hash */
  NameDef(TString) **p = G(L)->strcache[i];
  if (strcmp(str, getstr(p[0])) == 0)  /* hit? */
    return p[0];  /* that it is */
  else {  /* normal route */
    NameDef(TString) *s = NameDef(luaS_newlstr)(L, str, strlen(str));
    p[0] = s;
    return s;
  }
}


NameDef(Udata) * NameDef(luaS_newudata) (NameDef(lua_State) *L, size_t s) {
  NameDef(Udata) *u;
  NameDef(GCObject) *o;
  if (s > MAX_SIZE - sizeof(NameDef(Udata)))
    NameDef(luaM_toobig)(L);
  o = NameDef(luaC_newobj)(L, LUA_TUSERDATA, sizeludata(s));
  u = gco2u(o);
  u->len = s;
  u->metatable = NULL;
  setuservalue(L, u, luaO_nilobject);
  return u;
}

