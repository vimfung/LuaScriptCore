/*
** $Id: lundump.c,v 2.44 2015/11/02 16:09:30 roberto Exp $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"
#include "lundump.h"
#include "lzio.h"


#if !defined(luai_verifycode)
#define luai_verifycode(L,b,f)  /* empty */
#endif


typedef struct {
  NameDef(lua_State) *L;
  NameDef(ZIO) *Z;
  const char *name;
} NameDef(LoadState);


static l_noret error(NameDef(LoadState) *S, const char *why) {
  NameDef(luaO_pushfstring)(S->L, "%s: %s precompiled chunk", S->name, why);
  NameDef(luaD_throw)(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through LoadVector; you can change it to
** adapt to the endianness of the input
*/
#define LoadVector(S,b,n)	LoadBlock(S,b,(n)*sizeof((b)[0]))

static void LoadBlock (NameDef(LoadState) *S, void *b, size_t size) {
  if (NameDef(luaZ_read)(S->Z, b, size) != 0)
    error(S, "truncated");
}


#define LoadVar(S,x)		LoadVector(S,&x,1)


static NameDef(lu_byte) LoadByte (NameDef(LoadState) *S) {
  NameDef(lu_byte) x;
  LoadVar(S, x);
  return x;
}


static int LoadInt (NameDef(LoadState) *S) {
  int x;
  LoadVar(S, x);
  return x;
}


static NameDef(lua_Number) LoadNumber (NameDef(LoadState) *S) {
  NameDef(lua_Number) x;
  LoadVar(S, x);
  return x;
}


static NameDef(lua_Integer) LoadInteger (NameDef(LoadState) *S) {
  NameDef(lua_Integer) x;
  LoadVar(S, x);
  return x;
}


static NameDef(TString) *LoadString (NameDef(LoadState) *S) {
  size_t size = LoadByte(S);
  if (size == 0xFF)
    LoadVar(S, size);
  if (size == 0)
    return NULL;
  else if (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    char buff[LUAI_MAXSHORTLEN];
    LoadVector(S, buff, size);
    return NameDef(luaS_newlstr)(S->L, buff, size);
  }
  else {  /* long string */
    NameDef(TString) *ts = NameDef(luaS_createlngstrobj)(S->L, size);
    LoadVector(S, getstr(ts), size);  /* load directly in final place */
    return ts;
  }
}


static void LoadCode (NameDef(LoadState) *S, NameDef(Proto) *f) {
  int n = LoadInt(S);
  f->code = luaM_newvector(S->L, n, NameDef(Instruction));
  f->sizecode = n;
  LoadVector(S, f->code, n);
}


static void LoadFunction(NameDef(LoadState) *S, NameDef(Proto) *f, NameDef(TString) *psource);


static void LoadConstants (NameDef(LoadState) *S, NameDef(Proto) *f) {
  int i;
  int n = LoadInt(S);
  f->k = luaM_newvector(S->L, n, NameDef(TValue));
  f->sizek = n;
  for (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  for (i = 0; i < n; i++) {
    NameDef(TValue) *o = &f->k[i];
    int t = LoadByte(S);
    switch (t) {
    case LUA_TNIL:
      setnilvalue(o);
      break;
    case LUA_TBOOLEAN:
      setbvalue(o, LoadByte(S));
      break;
    case LUA_TNUMFLT:
      setfltvalue(o, LoadNumber(S));
      break;
    case LUA_TNUMINT:
      setivalue(o, LoadInteger(S));
      break;
    case LUA_TSHRSTR:
    case LUA_TLNGSTR:
      setsvalue2n(S->L, o, LoadString(S));
      break;
    default:
      lua_assert(0);
    }
  }
}


static void LoadProtos (NameDef(LoadState) *S, NameDef(Proto) *f) {
  int i;
  int n = LoadInt(S);
  f->p = luaM_newvector(S->L, n, NameDef(Proto) *);
  f->sizep = n;
  for (i = 0; i < n; i++)
    f->p[i] = NULL;
  for (i = 0; i < n; i++) {
    f->p[i] = NameDef(luaF_newproto)(S->L);
    LoadFunction(S, f->p[i], f->source);
  }
}


static void LoadUpvalues (NameDef(LoadState) *S, NameDef(Proto) *f) {
  int i, n;
  n = LoadInt(S);
  f->upvalues = luaM_newvector(S->L, n, NameDef(Upvaldesc));
  f->sizeupvalues = n;
  for (i = 0; i < n; i++)
    f->upvalues[i].name = NULL;
  for (i = 0; i < n; i++) {
    f->upvalues[i].instack = LoadByte(S);
    f->upvalues[i].idx = LoadByte(S);
  }
}


static void LoadDebug (NameDef(LoadState) *S, NameDef(Proto) *f) {
  int i, n;
  n = LoadInt(S);
  f->lineinfo = luaM_newvector(S->L, n, int);
  f->sizelineinfo = n;
  LoadVector(S, f->lineinfo, n);
  n = LoadInt(S);
  f->locvars = luaM_newvector(S->L, n, NameDef(LocVar));
  f->sizelocvars = n;
  for (i = 0; i < n; i++)
    f->locvars[i].varname = NULL;
  for (i = 0; i < n; i++) {
    f->locvars[i].varname = LoadString(S);
    f->locvars[i].startpc = LoadInt(S);
    f->locvars[i].endpc = LoadInt(S);
  }
  n = LoadInt(S);
  for (i = 0; i < n; i++)
    f->upvalues[i].name = LoadString(S);
}


static void LoadFunction (NameDef(LoadState) *S, NameDef(Proto) *f, NameDef(TString) *psource) {
  f->source = LoadString(S);
  if (f->source == NULL)  /* no source in dump? */
    f->source = psource;  /* reuse parent's source */
  f->linedefined = LoadInt(S);
  f->lastlinedefined = LoadInt(S);
  f->numparams = LoadByte(S);
  f->is_vararg = LoadByte(S);
  f->maxstacksize = LoadByte(S);
  LoadCode(S, f);
  LoadConstants(S, f);
  LoadUpvalues(S, f);
  LoadProtos(S, f);
  LoadDebug(S, f);
}


static void checkliteral (NameDef(LoadState) *S, const char *s, const char *msg) {
  char buff[sizeof(LUA_SIGNATURE) + sizeof(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  LoadVector(S, buff, len);
  if (memcmp(s, buff, len) != 0)
    error(S, msg);
}


static void fchecksize (NameDef(LoadState) *S, size_t size, const char *tname) {
  if (LoadByte(S) != size)
    error(S, NameDef(luaO_pushfstring)(S->L, "%s size mismatch in", tname));
}


#define checksize(S,t)	fchecksize(S,sizeof(t),#t)

static void checkHeader (NameDef(LoadState) *S) {
  checkliteral(S, LUA_SIGNATURE + 1, "not a");  /* 1st char already checked */
  if (LoadByte(S) != LUAC_VERSION)
    error(S, "version mismatch in");
  if (LoadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch in");
  checkliteral(S, LUAC_DATA, "corrupted");
  checksize(S, int);
  checksize(S, size_t);
  checksize(S, NameDef(Instruction));
  checksize(S, NameDef(lua_Integer));
  checksize(S, NameDef(lua_Number));
  if (LoadInteger(S) != LUAC_INT)
    error(S, "endianness mismatch in");
  if (LoadNumber(S) != LUAC_NUM)
    error(S, "float format mismatch in");
}


/*
** load precompiled chunk
*/
NameDef(LClosure) *NameDef(luaU_undump)(NameDef(lua_State) *L, NameDef(ZIO) *Z, const char *name) {
  NameDef(LoadState) S;
  NameDef(LClosure) *cl;
  if (*name == '@' || *name == '=')
    S.name = name + 1;
  else if (*name == LUA_SIGNATURE[0])
    S.name = "binary string";
  else
    S.name = name;
  S.L = L;
  S.Z = Z;
  checkHeader(&S);
  cl = NameDef(luaF_newLclosure)(L, LoadByte(&S));
  setclLvalue(L, L->top, cl);
  NameDef(luaD_inctop)(L);
  cl->p = NameDef(luaF_newproto)(L);
  LoadFunction(&S, cl->p, NULL);
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luai_verifycode(L, buff, cl->p);
  return cl;
}

