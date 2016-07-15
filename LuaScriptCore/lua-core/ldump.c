/*
** $Id: ldump.c,v 2.36 2015/03/30 15:43:51 roberto Exp $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#include "LuaDefine.h"

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "lundump.h"


typedef struct {
  NameDef(lua_State) *L;
  NameDef(lua_Writer) writer;
  void *data;
  int strip;
  int status;
} NameDef(DumpState);


/*
** All high-level dumps go through DumpVector; you can change it to
** change the endianness of the result
*/
#define DumpVector(v,n,D)	DumpBlock(v,(n)*sizeof((v)[0]),D)

#define DumpLiteral(s,D)	DumpBlock(s, sizeof(s) - sizeof(char), D)


static void DumpBlock (const void *b, size_t size, NameDef(DumpState) *D) {
  if (D->status == 0) {
    lua_unlock(D->L);
    D->status = (*D->writer)(D->L, b, size, D->data);
    lua_lock(D->L);
  }
}


#define DumpVar(x,D)		DumpVector(&x,1,D)


static void DumpByte (int y, NameDef(DumpState) *D) {
  NameDef(lu_byte) x = (NameDef(lu_byte))y;
  DumpVar(x, D);
}


static void DumpInt (int x, NameDef(DumpState) *D) {
  DumpVar(x, D);
}


static void DumpNumber (NameDef(lua_Number) x, NameDef(DumpState) *D) {
  DumpVar(x, D);
}


static void DumpInteger (NameDef(lua_Integer) x, NameDef(DumpState) *D) {
  DumpVar(x, D);
}


static void DumpString (const NameDef(TString) *s, NameDef(DumpState) *D) {
  if (s == NULL)
    DumpByte(0, D);
  else {
    size_t size = tsslen(s) + 1;  /* include trailing '\0' */
    const char *str = getstr(s);
    if (size < 0xFF)
      DumpByte(cast_int(size), D);
    else {
      DumpByte(0xFF, D);
      DumpVar(size, D);
    }
    DumpVector(str, size - 1, D);  /* no need to save '\0' */
  }
}


static void DumpCode (const NameDef(Proto) *f, NameDef(DumpState) *D) {
  DumpInt(f->sizecode, D);
  DumpVector(f->code, f->sizecode, D);
}


static void DumpFunction(const NameDef(Proto) *f, NameDef(TString) *psource, NameDef(DumpState) *D);

static void DumpConstants (const NameDef(Proto) *f, NameDef(DumpState) *D) {
  int i;
  int n = f->sizek;
  DumpInt(n, D);
  for (i = 0; i < n; i++) {
    const NameDef(TValue) *o = &f->k[i];
    DumpByte(ttype(o), D);
    switch (ttype(o)) {
    case LUA_TNIL:
      break;
    case LUA_TBOOLEAN:
      DumpByte(bvalue(o), D);
      break;
    case LUA_TNUMFLT:
      DumpNumber(fltvalue(o), D);
      break;
    case LUA_TNUMINT:
      DumpInteger(ivalue(o), D);
      break;
    case LUA_TSHRSTR:
    case LUA_TLNGSTR:
      DumpString(tsvalue(o), D);
      break;
    default:
      lua_assert(0);
    }
  }
}


static void DumpProtos (const NameDef(Proto) *f, NameDef(DumpState) *D) {
  int i;
  int n = f->sizep;
  DumpInt(n, D);
  for (i = 0; i < n; i++)
    DumpFunction(f->p[i], f->source, D);
}


static void DumpUpvalues (const NameDef(Proto) *f, NameDef(DumpState) *D) {
  int i, n = f->sizeupvalues;
  DumpInt(n, D);
  for (i = 0; i < n; i++) {
    DumpByte(f->upvalues[i].instack, D);
    DumpByte(f->upvalues[i].idx, D);
  }
}


static void DumpDebug (const NameDef(Proto) *f, NameDef(DumpState) *D) {
  int i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  DumpInt(n, D);
  DumpVector(f->lineinfo, n, D);
  n = (D->strip) ? 0 : f->sizelocvars;
  DumpInt(n, D);
  for (i = 0; i < n; i++) {
    DumpString(f->locvars[i].varname, D);
    DumpInt(f->locvars[i].startpc, D);
    DumpInt(f->locvars[i].endpc, D);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  DumpInt(n, D);
  for (i = 0; i < n; i++)
    DumpString(f->upvalues[i].name, D);
}


static void DumpFunction (const NameDef(Proto) *f, NameDef(TString) *psource, NameDef(DumpState) *D) {
  if (D->strip || f->source == psource)
    DumpString(NULL, D);  /* no debug info or same source as its parent */
  else
    DumpString(f->source, D);
  DumpInt(f->linedefined, D);
  DumpInt(f->lastlinedefined, D);
  DumpByte(f->numparams, D);
  DumpByte(f->is_vararg, D);
  DumpByte(f->maxstacksize, D);
  DumpCode(f, D);
  DumpConstants(f, D);
  DumpUpvalues(f, D);
  DumpProtos(f, D);
  DumpDebug(f, D);
}


static void DumpHeader (NameDef(DumpState) *D) {
  DumpLiteral(LUA_SIGNATURE, D);
  DumpByte(LUAC_VERSION, D);
  DumpByte(LUAC_FORMAT, D);
  DumpLiteral(LUAC_DATA, D);
  DumpByte(sizeof(int), D);
  DumpByte(sizeof(size_t), D);
  DumpByte(sizeof(NameDef(Instruction)), D);
  DumpByte(sizeof(NameDef(lua_Integer)), D);
  DumpByte(sizeof(NameDef(lua_Number)), D);
  DumpInteger(LUAC_INT, D);
  DumpNumber(LUAC_NUM, D);
}


/*
** dump Lua function as precompiled chunk
*/
int NameDef(luaU_dump)(NameDef(lua_State) *L, const NameDef(Proto) *f, NameDef(lua_Writer) w, void *data,
              int strip) {
  NameDef(DumpState) D;
  D.L = L;
  D.writer = w;
  D.data = data;
  D.strip = strip;
  D.status = 0;
  DumpHeader(&D);
  DumpByte(f->sizeupvalues, &D);
  DumpFunction(f, NULL, &D);
  return D.status;
}

