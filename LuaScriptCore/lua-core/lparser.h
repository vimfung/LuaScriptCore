/*
** $Id: lparser.h,v 1.74 2014/10/25 11:50:46 roberto Exp $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ifndef lparser_h
#define lparser_h

#include "LuaDefine.h"

#include "llimits.h"
#include "lobject.h"
#include "lzio.h"


/*
** Expression descriptor
*/

typedef enum {
  NameDef(VVOID),	/* no value */
  NameDef(VNIL),
  NameDef(VTRUE),
  NameDef(VFALSE),
  NameDef(VK),		/* info = index of constant in 'k' */
  NameDef(VKFLT),	/* nval = numerical float value */
  NameDef(VKINT),	/* nval = numerical integer value */
  NameDef(VNONRELOC),	/* info = result register */
  NameDef(VLOCAL),	/* info = local register */
  NameDef(VUPVAL),       /* info = index of upvalue in 'upvalues' */
  NameDef(VINDEXED),	/* t = table register/upvalue; idx = index R/K */
  NameDef(VJMP),		/* info = instruction pc */
  NameDef(VRELOCABLE),	/* info = instruction pc */
  NameDef(VCALL),	/* info = instruction pc */
  NameDef(VVARARG)	/* info = instruction pc */
} NameDef(expkind);


#define vkisvar(k)	(NameDef(VLOCAL) <= (k) && (k) <= NameDef(VINDEXED))
#define vkisinreg(k)	((k) == NameDef(VNONRELOC) || (k) == NameDef(VLOCAL))

typedef struct NameDef(expdesc) {
  NameDef(expkind) k;
  union {
    struct {  /* for indexed variables (VINDEXED) */
      short idx;  /* index (R/K) */
      NameDef(lu_byte) t;  /* table (register or upvalue) */
      NameDef(lu_byte) vt;  /* whether 't' is register (VLOCAL) or upvalue (VUPVAL) */
    } ind;
    int info;  /* for generic use */
    NameDef(lua_Number) nval;  /* for VKFLT */
    NameDef(lua_Integer) ival;    /* for VKINT */
  } u;
  int t;  /* patch list of 'exit when true' */
  int f;  /* patch list of 'exit when false' */
} NameDef(expdesc);


/* description of active local variable */
typedef struct NameDef(Vardesc) {
  short idx;  /* variable index in stack */
} NameDef(Vardesc);


/* description of pending goto statements and label statements */
typedef struct NameDef(Labeldesc) {
  NameDef(TString) *name;  /* label identifier */
  int pc;  /* position in code */
  int line;  /* line where it appeared */
  NameDef(lu_byte) nactvar;  /* local level where it appears in current block */
} NameDef(Labeldesc);


/* list of labels or gotos */
typedef struct NameDef(Labellist) {
  NameDef(Labeldesc) *arr;  /* array */
  int n;  /* number of entries in use */
  int size;  /* array size */
} NameDef(Labellist);


/* dynamic structures used by the parser */
typedef struct NameDef(Dyndata) {
  struct {  /* list of active local variables */
    NameDef(Vardesc) *arr;
    int n;
    int size;
  } actvar;
  NameDef(Labellist) gt;  /* list of pending gotos */
  NameDef(Labellist) label;   /* list of active labels */
} NameDef(Dyndata);


/* control of blocks */
struct NameDef(BlockCnt);  /* defined in lparser.c */


/* state needed to generate code for a given function */
typedef struct NameDef(FuncState) {
  NameDef(Proto) *f;  /* current function header */
  struct NameDef(FuncState) *prev;  /* enclosing function */
  struct NameDef(LexState) *ls;  /* lexical state */
  struct NameDef(BlockCnt) *bl;  /* chain of current blocks */
  int pc;  /* next position to code (equivalent to 'ncode') */
  int lasttarget;   /* 'label' of last 'jump label' */
  int jpc;  /* list of pending jumps to 'pc' */
  int nk;  /* number of elements in 'k' */
  int np;  /* number of elements in 'p' */
  int firstlocal;  /* index of first local var (in Dyndata array) */
  short nlocvars;  /* number of elements in 'f->locvars' */
  NameDef(lu_byte) nactvar;  /* number of active local variables */
  NameDef(lu_byte) nups;  /* number of upvalues */
  NameDef(lu_byte) freereg;  /* first free register */
} NameDef(FuncState);


LUAI_FUNC NameDef(LClosure) *NameDef(luaY_parser) (NameDef(lua_State) *L, NameDef(ZIO) *z, NameDef(Mbuffer) *buff,
                                 NameDef(Dyndata) *dyd, const char *name, int firstchar);


#endif
