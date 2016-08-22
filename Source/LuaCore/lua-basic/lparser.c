/*
** $Id: lparser.c,v 2.153 2016/05/13 19:10:16 roberto Exp $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "llex.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"



/* maximum number of local variables per function (must be smaller
   than 250, due to the bytecode format) */
#define MAXVARS		200


#define hasmultret(k)		((k) == NameDef(VCALL) || (k) == NameDef(VVARARG))


/* because all strings are unified by the scanner, the parser
   can use pointer equality for string equality */
#define eqstr(a,b)	((a) == (b))


/*
** nodes for block list (list of active blocks)
*/
typedef struct NameDef(BlockCnt) {
  struct NameDef(BlockCnt) *previous;  /* chain */
  int firstlabel;  /* index of first label in this block */
  int firstgoto;  /* index of first pending goto in this block */
  NameDef(lu_byte) nactvar;  /* # active locals outside the block */
  NameDef(lu_byte) upval;  /* true if some variable in the block is an upvalue */
  NameDef(lu_byte) isloop;  /* true if 'block' is a loop */
} NameDef(BlockCnt);



/*
** prototypes for recursive non-terminal functions
*/
static void statement (NameDef(LexState) *ls);
static void expr (NameDef(LexState) *ls, NameDef(expdesc) *v);


/* semantic error */
static l_noret semerror (NameDef(LexState) *ls, const char *msg) {
  ls->t.token = 0;  /* remove "near <token>" from final message */
  NameDef(luaX_syntaxerror)(ls, msg);
}


static l_noret error_expected (NameDef(LexState) *ls, int token) {
  NameDef(luaX_syntaxerror)(ls,
      NameDef(luaO_pushfstring)(ls->L, "%s expected", NameDef(luaX_token2str)(ls, token)));
}


static l_noret errorlimit (NameDef(FuncState) *fs, int limit, const char *what) {
  NameDef(lua_State) *L = fs->ls->L;
  const char *msg;
  int line = fs->f->linedefined;
  const char *where = (line == 0)
                      ? "main function"
                      : NameDef(luaO_pushfstring)(L, "function at line %d", line);
  msg = NameDef(luaO_pushfstring)(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  NameDef(luaX_syntaxerror)(fs->ls, msg);
}


static void checklimit (NameDef(FuncState) *fs, int v, int l, const char *what) {
  if (v > l) errorlimit(fs, l, what);
}


static int testnext (NameDef(LexState) *ls, int c) {
  if (ls->t.token == c) {
    NameDef(luaX_next)(ls);
    return 1;
  }
  else return 0;
}


static void check (NameDef(LexState) *ls, int c) {
  if (ls->t.token != c)
    error_expected(ls, c);
}


static void checknext (NameDef(LexState) *ls, int c) {
  check(ls, c);
  NameDef(luaX_next)(ls);
}


#define check_condition(ls,c,msg)	{ if (!(c)) NameDef(luaX_syntaxerror)(ls, msg); }



static void check_match (NameDef(LexState) *ls, int what, int who, int where) {
  if (!testnext(ls, what)) {
    if (where == ls->linenumber)
      error_expected(ls, what);
    else {
      NameDef(luaX_syntaxerror)(ls, NameDef(luaO_pushfstring)(ls->L,
             "%s expected (to close %s at line %d)",
              NameDef(luaX_token2str)(ls, what), NameDef(luaX_token2str)(ls, who), where));
    }
  }
}


static NameDef(TString) *str_checkname (NameDef(LexState) *ls) {
  NameDef(TString) *ts;
  check(ls, NameDef(TK_NAME));
  ts = ls->t.seminfo.ts;
  NameDef(luaX_next)(ls);
  return ts;
}


static void init_exp (NameDef(expdesc) *e, NameDef(expkind) k, int i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


static void codestring (NameDef(LexState) *ls, NameDef(expdesc) *e, NameDef(TString) *s) {
  init_exp(e, NameDef(VK), NameDef(luaK_stringK)(ls->fs, s));
}


static void checkname (NameDef(LexState) *ls, NameDef(expdesc) *e) {
  codestring(ls, e, str_checkname(ls));
}


static int registerlocalvar (NameDef(LexState) *ls, NameDef(TString) *varname) {
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Proto) *f = fs->f;
  int oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->nlocvars, f->sizelocvars,
                  NameDef(LocVar), SHRT_MAX, "local variables");
  while (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = NULL;
  f->locvars[fs->nlocvars].varname = varname;
  luaC_objbarrier(ls->L, f, varname);
  return fs->nlocvars++;
}


static void new_localvar (NameDef(LexState) *ls, NameDef(TString) *name) {
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Dyndata) *dyd = ls->dyd;
  int reg = registerlocalvar(ls, name);
  checklimit(fs, dyd->actvar.n + 1 - fs->firstlocal,
                  MAXVARS, "local variables");
  luaM_growvector(ls->L, dyd->actvar.arr, dyd->actvar.n + 1,
                  dyd->actvar.size, NameDef(Vardesc), MAX_INT, "local variables");
  dyd->actvar.arr[dyd->actvar.n++].idx = cast(short, reg);
}


static void new_localvarliteral_ (NameDef(LexState) *ls, const char *name, size_t sz) {
  new_localvar(ls, NameDef(luaX_newstring)(ls, name, sz));
}

#define new_localvarliteral(ls,v) \
	new_localvarliteral_(ls, "" v, (sizeof(v)/sizeof(char))-1)


static NameDef(LocVar) *getlocvar (NameDef(FuncState) *fs, int i) {
  int idx = fs->ls->dyd->actvar.arr[fs->firstlocal + i].idx;
  lua_assert(idx < fs->nlocvars);
  return &fs->f->locvars[idx];
}


static void adjustlocalvars (NameDef(LexState) *ls, int nvars) {
  NameDef(FuncState) *fs = ls->fs;
  fs->nactvar = cast_byte(fs->nactvar + nvars);
  for (; nvars; nvars--) {
    getlocvar(fs, fs->nactvar - nvars)->startpc = fs->pc;
  }
}


static void removevars (NameDef(FuncState) *fs, int tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  while (fs->nactvar > tolevel)
    getlocvar(fs, --fs->nactvar)->endpc = fs->pc;
}


static int searchupvalue (NameDef(FuncState) *fs, NameDef(TString) *name) {
  int i;
  NameDef(Upvaldesc) *up = fs->f->upvalues;
  for (i = 0; i < fs->nups; i++) {
    if (eqstr(up[i].name, name)) return i;
  }
  return -1;  /* not found */
}


static int newupvalue (NameDef(FuncState) *fs, NameDef(TString) *name, NameDef(expdesc) *v) {
  NameDef(Proto) *f = fs->f;
  int oldsize = f->sizeupvalues;
  checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  NameDef(Upvaldesc), MAXUPVAL, "upvalues");
  while (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = NULL;
  f->upvalues[fs->nups].instack = (v->k == NameDef(VLOCAL));
  f->upvalues[fs->nups].idx = cast_byte(v->u.info);
  f->upvalues[fs->nups].name = name;
  luaC_objbarrier(fs->ls->L, f, name);
  return fs->nups++;
}


static int searchvar (NameDef(FuncState) *fs, NameDef(TString) *n) {
  int i;
  for (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    if (eqstr(n, getlocvar(fs, i)->varname))
      return i;
  }
  return -1;  /* not found */
}


/*
  Mark block where variable at given level was defined
  (to emit close instructions later).
*/
static void markupval (NameDef(FuncState) *fs, int level) {
  NameDef(BlockCnt) *bl = fs->bl;
  while (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
}


/*
  Find variable with given name 'n'. If it is an upvalue, add this
  upvalue into all intermediate functions.
*/
static void singlevaraux (NameDef(FuncState) *fs, NameDef(TString) *n, NameDef(expdesc) *var, int base) {
  if (fs == NULL)  /* no more levels? */
    init_exp(var, NameDef(VVOID), 0);  /* default is global */
  else {
    int v = searchvar(fs, n);  /* look up locals at current level */
    if (v >= 0) {  /* found? */
      init_exp(var, NameDef(VLOCAL), v);  /* variable is local */
      if (!base)
        markupval(fs, v);  /* local will be used as an upval */
    }
    else {  /* not found as local at current level; try upvalues */
      int idx = searchupvalue(fs, n);  /* try existing upvalues */
      if (idx < 0) {  /* not found? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
        if (var->k == NameDef(VVOID))  /* not found? */
          return;  /* it is a global */
        /* else was LOCAL or UPVAL */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      }
      init_exp(var, NameDef(VUPVAL), idx);  /* new or old upvalue */
    }
  }
}


static void singlevar (NameDef(LexState) *ls, NameDef(expdesc) *var) {
  NameDef(TString) *varname = str_checkname(ls);
  NameDef(FuncState) *fs = ls->fs;
  singlevaraux(fs, varname, var, 1);
  if (var->k == NameDef(VVOID)) {  /* global name? */
    NameDef(expdesc) key;
    singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
    lua_assert(var->k != NameDef(VVOID));  /* this one must exist */
    codestring(ls, &key, varname);  /* key is variable name */
    NameDef(luaK_indexed)(fs, var, &key);  /* env[varname] */
  }
}


static void adjust_assign (NameDef(LexState) *ls, int nvars, int nexps, NameDef(expdesc) *e) {
  NameDef(FuncState) *fs = ls->fs;
  int extra = nvars - nexps;
  if (hasmultret(e->k)) {
    extra++;  /* includes call itself */
    if (extra < 0) extra = 0;
    NameDef(luaK_setreturns)(fs, e, extra);  /* last exp. provides the difference */
    if (extra > 1) NameDef(luaK_reserveregs)(fs, extra-1);
  }
  else {
    if (e->k != NameDef(VVOID)) NameDef(luaK_exp2nextreg)(fs, e);  /* close last expression */
    if (extra > 0) {
      int reg = fs->freereg;
      NameDef(luaK_reserveregs)(fs, extra);
      NameDef(luaK_nil)(fs, reg, extra);
    }
  }
}


static void enterlevel (NameDef(LexState) *ls) {
  NameDef(lua_State) *L = ls->L;
  ++L->nCcalls;
  checklimit(ls->fs, L->nCcalls, LUAI_MAXCCALLS, "C levels");
}


#define leavelevel(ls)	((ls)->L->nCcalls--)


static void closegoto (NameDef(LexState) *ls, int g, NameDef(Labeldesc) *label) {
  int i;
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Labellist) *gl = &ls->dyd->gt;
  NameDef(Labeldesc) *gt = &gl->arr[g];
  lua_assert(eqstr(gt->name, label->name));
  if (gt->nactvar < label->nactvar) {
    NameDef(TString) *vname = getlocvar(fs, gt->nactvar)->varname;
    const char *msg = NameDef(luaO_pushfstring)(ls->L,
      "<goto %s> at line %d jumps into the scope of local '%s'",
      getstr(gt->name), gt->line, getstr(vname));
    semerror(ls, msg);
  }
  NameDef(luaK_patchlist)(fs, gt->pc, label->pc);
  /* remove goto from pending list */
  for (i = g; i < gl->n - 1; i++)
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** try to close a goto with existing labels; this solves backward jumps
*/
static int findlabel (NameDef(LexState) *ls, int g) {
  int i;
  NameDef(BlockCnt) *bl = ls->fs->bl;
  NameDef(Dyndata) *dyd = ls->dyd;
  NameDef(Labeldesc) *gt = &dyd->gt.arr[g];
  /* check labels in current block for a match */
  for (i = bl->firstlabel; i < dyd->label.n; i++) {
    NameDef(Labeldesc) *lb = &dyd->label.arr[i];
    if (eqstr(lb->name, gt->name)) {  /* correct label? */
      if (gt->nactvar > lb->nactvar &&
          (bl->upval || dyd->label.n > bl->firstlabel))
        NameDef(luaK_patchclose)(ls->fs, gt->pc, lb->nactvar);
      closegoto(ls, g, lb);  /* close it */
      return 1;
    }
  }
  return 0;  /* label not found; cannot close goto */
}


static int newlabelentry (NameDef(LexState) *ls, NameDef(Labellist) *l, NameDef(TString) *name,
                          int line, int pc) {
  int n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  NameDef(Labeldesc), SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].line = line;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].pc = pc;
  l->n = n + 1;
  return n;
}


/*
** check whether new label 'lb' matches any pending gotos in current
** block; solves forward jumps
*/
static void findgotos (NameDef(LexState) *ls, NameDef(Labeldesc) *lb) {
  NameDef(Labellist) *gl = &ls->dyd->gt;
  int i = ls->fs->bl->firstgoto;
  while (i < gl->n) {
    if (eqstr(gl->arr[i].name, lb->name))
      closegoto(ls, i, lb);
    else
      i++;
  }
}


/*
** export pending gotos to outer level, to check them against
** outer labels; if the block being exited has upvalues, and
** the goto exits the scope of any variable (which can be the
** upvalue), close those variables being exited.
*/
static void movegotosout (NameDef(FuncState) *fs, NameDef(BlockCnt) *bl) {
  int i = bl->firstgoto;
  NameDef(Labellist) *gl = &fs->ls->dyd->gt;
  /* correct pending gotos to current block and try to close it
     with visible labels */
  while (i < gl->n) {
    NameDef(Labeldesc) *gt = &gl->arr[i];
    if (gt->nactvar > bl->nactvar) {
      if (bl->upval)
        NameDef(luaK_patchclose)(fs, gt->pc, bl->nactvar);
      gt->nactvar = bl->nactvar;
    }
    if (!findlabel(fs->ls, i))
      i++;  /* move to next one */
  }
}


static void enterblock (NameDef(FuncState) *fs, NameDef(BlockCnt) *bl, NameDef(lu_byte) isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  bl->previous = fs->bl;
  fs->bl = bl;
  lua_assert(fs->freereg == fs->nactvar);
}


/*
** create a label named 'break' to resolve break statements
*/
static void breaklabel (NameDef(LexState) *ls) {
  NameDef(TString) *n = NameDef(luaS_new)(ls->L, "break");
  int l = newlabelentry(ls, &ls->dyd->label, n, 0, ls->fs->pc);
  findgotos(ls, &ls->dyd->label.arr[l]);
}

/*
** generates an error for an undefined 'goto'; choose appropriate
** message when label name is a reserved word (which can only be 'break')
*/
static l_noret undefgoto (NameDef(LexState) *ls, NameDef(Labeldesc) *gt) {
  const char *msg = isreserved(gt->name)
                    ? "<%s> at line %d not inside a loop"
                    : "no visible label '%s' for <goto> at line %d";
  msg = NameDef(luaO_pushfstring)(ls->L, msg, getstr(gt->name), gt->line);
  semerror(ls, msg);
}


static void leaveblock (NameDef(FuncState) *fs) {
  NameDef(BlockCnt) *bl = fs->bl;
  NameDef(LexState) *ls = fs->ls;
  if (bl->previous && bl->upval) {
    /* create a 'jump to here' to close upvalues */
    int j = NameDef(luaK_jump)(fs);
    NameDef(luaK_patchclose)(fs, j, bl->nactvar);
    NameDef(luaK_patchtohere)(fs, j);
  }
  if (bl->isloop)
    breaklabel(ls);  /* close pending breaks */
  fs->bl = bl->previous;
  removevars(fs, bl->nactvar);
  lua_assert(bl->nactvar == fs->nactvar);
  fs->freereg = fs->nactvar;  /* free registers */
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
  if (bl->previous)  /* inner block? */
    movegotosout(fs, bl);  /* update pending gotos to outer block */
  else if (bl->firstgoto < ls->dyd->gt.n)  /* pending gotos in outer block? */
    undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
}


/*
** adds a new prototype into list of prototypes
*/
static NameDef(Proto) *addprototype (NameDef(LexState) *ls) {
  NameDef(Proto) *clp;
  NameDef(lua_State) *L = ls->L;
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Proto) *f = fs->f;  /* prototype of current function */
  if (fs->np >= f->sizep) {
    int oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, NameDef(Proto) *, MAXARG_Bx, "functions");
    while (oldsize < f->sizep)
      f->p[oldsize++] = NULL;
  }
  f->p[fs->np++] = clp = NameDef(luaF_newproto)(L);
  luaC_objbarrier(L, f, clp);
  return clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction must use the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.
*/
static void codeclosure (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  NameDef(FuncState) *fs = ls->fs->prev;
  init_exp(v, NameDef(VRELOCABLE), NameDef(luaK_codeABx)(fs, NameDef(OP_CLOSURE), 0, fs->np - 1));
  NameDef(luaK_exp2nextreg)(fs, v);  /* fix it at the last register */
}


static void open_func (NameDef(LexState) *ls, NameDef(FuncState) *fs, NameDef(BlockCnt) *bl) {
  NameDef(Proto) *f;
  fs->prev = ls->fs;  /* linked list of funcstates */
  fs->ls = ls;
  ls->fs = fs;
  fs->pc = 0;
  fs->lasttarget = 0;
  fs->jpc = NO_JUMP;
  fs->freereg = 0;
  fs->nk = 0;
  fs->np = 0;
  fs->nups = 0;
  fs->nlocvars = 0;
  fs->nactvar = 0;
  fs->firstlocal = ls->dyd->actvar.n;
  fs->bl = NULL;
  f = fs->f;
  f->source = ls->source;
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  enterblock(fs, bl, 0);
}


static void close_func (NameDef(LexState) *ls) {
  NameDef(lua_State) *L = ls->L;
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Proto) *f = fs->f;
  NameDef(luaK_ret)(fs, 0, 0);  /* final return */
  leaveblock(fs);
  luaM_reallocvector(L, f->code, f->sizecode, fs->pc, NameDef(Instruction));
  f->sizecode = fs->pc;
  luaM_reallocvector(L, f->lineinfo, f->sizelineinfo, fs->pc, int);
  f->sizelineinfo = fs->pc;
  luaM_reallocvector(L, f->k, f->sizek, fs->nk, NameDef(TValue));
  f->sizek = fs->nk;
  luaM_reallocvector(L, f->p, f->sizep, fs->np, NameDef(Proto) *);
  f->sizep = fs->np;
  luaM_reallocvector(L, f->locvars, f->sizelocvars, fs->nlocvars, NameDef(LocVar));
  f->sizelocvars = fs->nlocvars;
  luaM_reallocvector(L, f->upvalues, f->sizeupvalues, fs->nups, NameDef(Upvaldesc));
  f->sizeupvalues = fs->nups;
  lua_assert(fs->bl == NULL);
  ls->fs = fs->prev;
  luaC_checkGC(L);
}



/*============================================================*/
/* GRAMMAR RULES */
/*============================================================*/


/*
** check whether current token is in the follow set of a block.
** 'until' closes syntactical blocks, but do not close scope,
** so it is handled in separate.
*/
static int block_follow (NameDef(LexState) *ls, int withuntil) {
  switch (ls->t.token) {
    case NameDef(TK_ELSE): case NameDef(TK_ELSEIF):
    case NameDef(TK_END): case NameDef(TK_EOS):
      return 1;
    case NameDef(TK_UNTIL): return withuntil;
    default: return 0;
  }
}


static void statlist (NameDef(LexState) *ls) {
  /* statlist -> { stat [';'] } */
  while (!block_follow(ls, 1)) {
    if (ls->t.token == NameDef(TK_RETURN)) {
      statement(ls);
      return;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


static void fieldsel (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(expdesc) key;
  NameDef(luaK_exp2anyregup)(fs, v);
  NameDef(luaX_next)(ls);  /* skip the dot or colon */
  checkname(ls, &key);
  NameDef(luaK_indexed)(fs, v, &key);
}


static void yindex (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* index -> '[' expr ']' */
  NameDef(luaX_next)(ls);  /* skip the '[' */
  expr(ls, v);
  NameDef(luaK_exp2val)(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/


struct NameDef(ConsControl) {
  NameDef(expdesc) v;  /* last list item read */
  NameDef(expdesc) *t;  /* table descriptor */
  int nh;  /* total number of 'record' elements */
  int na;  /* total number of array elements */
  int tostore;  /* number of array elements pending to be stored */
};


static void recfield (NameDef(LexState) *ls, struct NameDef(ConsControl) *cc) {
  /* recfield -> (NAME | '['exp1']') = exp1 */
  NameDef(FuncState) *fs = ls->fs;
  int reg = ls->fs->freereg;
  NameDef(expdesc) key, val;
  int rkkey;
  if (ls->t.token == NameDef(TK_NAME)) {
    checklimit(fs, cc->nh, MAX_INT, "items in a constructor");
    checkname(ls, &key);
  }
  else  /* ls->t.token == '[' */
    yindex(ls, &key);
  cc->nh++;
  checknext(ls, '=');
  rkkey = NameDef(luaK_exp2RK)(fs, &key);
  expr(ls, &val);
  NameDef(luaK_codeABC)(fs, NameDef(OP_SETTABLE), cc->t->u.info, rkkey, NameDef(luaK_exp2RK)(fs, &val));
  fs->freereg = reg;  /* free registers */
}


static void closelistfield (NameDef(FuncState) *fs, struct NameDef(ConsControl) *cc) {
  if (cc->v.k == NameDef(VVOID)) return;  /* there is no list item */
  NameDef(luaK_exp2nextreg)(fs, &cc->v);
  cc->v.k = NameDef(VVOID);
  if (cc->tostore == LFIELDS_PER_FLUSH) {
    NameDef(luaK_setlist)(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->tostore = 0;  /* no more items pending */
  }
}


static void lastlistfield (NameDef(FuncState) *fs, struct NameDef(ConsControl) *cc) {
  if (cc->tostore == 0) return;
  if (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    NameDef(luaK_setlist)(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  else {
    if (cc->v.k != NameDef(VVOID))
      NameDef(luaK_exp2nextreg)(fs, &cc->v);
    NameDef(luaK_setlist)(fs, cc->t->u.info, cc->na, cc->tostore);
  }
}


static void listfield (NameDef(LexState) *ls, struct NameDef(ConsControl) *cc) {
  /* listfield -> exp */
  expr(ls, &cc->v);
  checklimit(ls->fs, cc->na, MAX_INT, "items in a constructor");
  cc->na++;
  cc->tostore++;
}


static void field (NameDef(LexState) *ls, struct NameDef(ConsControl) *cc) {
  /* field -> listfield | recfield */
  switch(ls->t.token) {
    case NameDef(TK_NAME): {  /* may be 'listfield' or 'recfield' */
      if (NameDef(luaX_lookahead)(ls) != '=')  /* expression? */
        listfield(ls, cc);
      else
        recfield(ls, cc);
      break;
    }
    case '[': {
      recfield(ls, cc);
      break;
    }
    default: {
      listfield(ls, cc);
      break;
    }
  }
}


static void constructor (NameDef(LexState) *ls, NameDef(expdesc) *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  NameDef(FuncState) *fs = ls->fs;
  int line = ls->linenumber;
  int pc = NameDef(luaK_codeABC)(fs, NameDef(OP_NEWTABLE), 0, 0, 0);
  struct NameDef(ConsControl) cc;
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, NameDef(VRELOCABLE), pc);
  init_exp(&cc.v, NameDef(VVOID), 0);  /* no value (yet) */
  NameDef(luaK_exp2nextreg)(ls->fs, t);  /* fix it at stack top */
  checknext(ls, '{');
  do {
    lua_assert(cc.v.k == VVOID || cc.tostore > 0);
    if (ls->t.token == '}') break;
    closelistfield(fs, &cc);
    field(ls, &cc);
  } while (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, '}', '{', line);
  lastlistfield(fs, &cc);
  SETARG_B(fs->f->code[pc], NameDef(luaO_int2fb)(cc.na)); /* set initial array size */
  SETARG_C(fs->f->code[pc], NameDef(luaO_int2fb)(cc.nh));  /* set initial table size */
}

/* }====================================================================== */



static void parlist (NameDef(LexState) *ls) {
  /* parlist -> [ param { ',' param } ] */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Proto) *f = fs->f;
  int nparams = 0;
  f->is_vararg = 0;
  if (ls->t.token != ')') {  /* is 'parlist' not empty? */
    do {
      switch (ls->t.token) {
        case NameDef(TK_NAME): {  /* param -> NAME */
          new_localvar(ls, str_checkname(ls));
          nparams++;
          break;
        }
        case NameDef(TK_DOTS): {  /* param -> '...' */
          NameDef(luaX_next)(ls);
          f->is_vararg = 2;  /* declared vararg */
          break;
        }
        default: NameDef(luaX_syntaxerror)(ls, "<name> or '...' expected");
      }
    } while (!f->is_vararg && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  NameDef(luaK_reserveregs)(fs, fs->nactvar);  /* reserve register for parameters */
}


static void body (NameDef(LexState) *ls, NameDef(expdesc) *e, int ismethod, int line) {
  /* body ->  '(' parlist ')' block END */
  NameDef(FuncState) new_fs;
  NameDef(BlockCnt) bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = line;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  if (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, NameDef(TK_END), NameDef(TK_FUNCTION), line);
  codeclosure(ls, e);
  close_func(ls);
}


static int explist (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* explist -> expr { ',' expr } */
  int n = 1;  /* at least one expression */
  expr(ls, v);
  while (testnext(ls, ',')) {
    NameDef(luaK_exp2nextreg)(ls->fs, v);
    expr(ls, v);
    n++;
  }
  return n;
}


static void funcargs (NameDef(LexState) *ls, NameDef(expdesc) *f, int line) {
  NameDef(FuncState) *fs = ls->fs;
  NameDef(expdesc) args;
  int base, nparams;
  switch (ls->t.token) {
    case '(': {  /* funcargs -> '(' [ explist ] ')' */
      NameDef(luaX_next)(ls);
      if (ls->t.token == ')')  /* arg list is empty? */
        args.k = NameDef(VVOID);
      else {
        explist(ls, &args);
        luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', line);
      break;
    }
    case '{': {  /* funcargs -> constructor */
      constructor(ls, &args);
      break;
    }
    case NameDef(TK_STRING): {  /* funcargs -> STRING */
      codestring(ls, &args, ls->t.seminfo.ts);
      NameDef(luaX_next)(ls);  /* must use 'seminfo' before 'next' */
      break;
    }
    default: {
      NameDef(luaX_syntaxerror)(ls, "function arguments expected");
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  if (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  else {
    if (args.k != NameDef(VVOID))
      NameDef(luaK_exp2nextreg)(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, NameDef(VCALL), NameDef(luaK_codeABC)(fs, NameDef(OP_CALL), base, nparams+1, 2));
  NameDef(luaK_fixline)(fs, line);
  fs->freereg = base+1;  /* call remove function and arguments and leaves
                            (unless changed) one result */
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


static void primaryexp (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  switch (ls->t.token) {
    case '(': {
      int line = ls->linenumber;
      NameDef(luaX_next)(ls);
      expr(ls, v);
      check_match(ls, ')', '(', line);
      NameDef(luaK_dischargevars)(ls->fs, v);
      return;
    }
    case NameDef(TK_NAME): {
      singlevar(ls, v);
      return;
    }
    default: {
      NameDef(luaX_syntaxerror)(ls, "unexpected symbol");
    }
  }
}


static void suffixedexp (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  NameDef(FuncState) *fs = ls->fs;
  int line = ls->linenumber;
  primaryexp(ls, v);
  for (;;) {
    switch (ls->t.token) {
      case '.': {  /* fieldsel */
        fieldsel(ls, v);
        break;
      }
      case '[': {  /* '[' exp1 ']' */
        NameDef(expdesc) key;
        NameDef(luaK_exp2anyregup)(fs, v);
        yindex(ls, &key);
        NameDef(luaK_indexed)(fs, v, &key);
        break;
      }
      case ':': {  /* ':' NAME funcargs */
        NameDef(expdesc) key;
        NameDef(luaX_next)(ls);
        checkname(ls, &key);
        NameDef(luaK_self)(fs, v, &key);
        funcargs(ls, v, line);
        break;
      }
      case '(': case NameDef(TK_STRING): case '{': {  /* funcargs */
        NameDef(luaK_exp2nextreg)(fs, v);
        funcargs(ls, v, line);
        break;
      }
      default: return;
    }
  }
}


static void simpleexp (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  switch (ls->t.token) {
    case NameDef(TK_FLT): {
      init_exp(v, NameDef(VKFLT), 0);
      v->u.nval = ls->t.seminfo.r;
      break;
    }
    case NameDef(TK_INT): {
      init_exp(v, NameDef(VKINT), 0);
      v->u.ival = ls->t.seminfo.i;
      break;
    }
    case NameDef(TK_STRING): {
      codestring(ls, v, ls->t.seminfo.ts);
      break;
    }
    case NameDef(TK_NIL): {
      init_exp(v, NameDef(VNIL), 0);
      break;
    }
    case NameDef(TK_TRUE): {
      init_exp(v, NameDef(VTRUE), 0);
      break;
    }
    case NameDef(TK_FALSE): {
      init_exp(v, NameDef(VFALSE), 0);
      break;
    }
    case NameDef(TK_DOTS): {  /* vararg */
      NameDef(FuncState) *fs = ls->fs;
      check_condition(ls, fs->f->is_vararg,
                      "cannot use '...' outside a vararg function");
      fs->f->is_vararg = 1;  /* function actually uses vararg */
      init_exp(v, NameDef(VVARARG), NameDef(luaK_codeABC)(fs, NameDef(OP_VARARG), 0, 1, 0));
      break;
    }
    case '{': {  /* constructor */
      constructor(ls, v);
      return;
    }
    case NameDef(TK_FUNCTION): {
      NameDef(luaX_next)(ls);
      body(ls, v, 0, ls->linenumber);
      return;
    }
    default: {
      suffixedexp(ls, v);
      return;
    }
  }
  NameDef(luaX_next)(ls);
}


static NameDef(UnOpr) getunopr (int op) {
  switch (op) {
    case NameDef(TK_NOT): return NameDef(OPR_NOT);
    case '-': return NameDef(OPR_MINUS);
    case '~': return NameDef(OPR_BNOT);
    case '#': return NameDef(OPR_LEN);
    default: return NameDef(OPR_NOUNOPR);
  }
}


static NameDef(BinOpr) getbinopr (int op) {
  switch (op) {
    case '+': return NameDef(OPR_ADD);
    case '-': return NameDef(OPR_SUB);
    case '*': return NameDef(OPR_MUL);
    case '%': return NameDef(OPR_MOD);
    case '^': return NameDef(OPR_POW);
    case '/': return NameDef(OPR_DIV);
    case NameDef(TK_IDIV): return NameDef(OPR_IDIV);
    case '&': return NameDef(OPR_BAND);
    case '|': return NameDef(OPR_BOR);
    case '~': return NameDef(OPR_BXOR);
    case NameDef(TK_SHL): return NameDef(OPR_SHL);
    case NameDef(TK_SHR): return NameDef(OPR_SHR);
    case NameDef(TK_CONCAT): return NameDef(OPR_CONCAT);
    case NameDef(TK_NE): return NameDef(OPR_NE);
    case NameDef(TK_EQ): return NameDef(OPR_EQ);
    case '<': return NameDef(OPR_LT);
    case NameDef(TK_LE): return NameDef(OPR_LE);
    case '>': return NameDef(OPR_GT);
    case NameDef(TK_GE): return NameDef(OPR_GE);
    case NameDef(TK_AND): return NameDef(OPR_AND);
    case NameDef(TK_OR): return NameDef(OPR_OR);
    default: return NameDef(OPR_NOBINOPR);
  }
}


static const struct {
  NameDef(lu_byte) left;  /* left priority for each binary operator */
  NameDef(lu_byte) right; /* right priority */
} priority[] = {  /* ORDER OPR */
   {10, 10}, {10, 10},           /* '+' '-' */
   {11, 11}, {11, 11},           /* '*' '%' */
   {14, 13},                  /* '^' (right associative) */
   {11, 11}, {11, 11},           /* '/' '//' */
   {6, 6}, {4, 4}, {5, 5},   /* '&' '|' '~' */
   {7, 7}, {7, 7},           /* '<<' '>>' */
   {9, 8},                   /* '..' (right associative) */
   {3, 3}, {3, 3}, {3, 3},   /* ==, <, <= */
   {3, 3}, {3, 3}, {3, 3},   /* ~=, >, >= */
   {2, 2}, {1, 1}            /* and, or */
};

#define UNARY_PRIORITY	12  /* priority for unary operators */


/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where 'binop' is any binary operator with a priority higher than 'limit'
*/
static NameDef(BinOpr) subexpr (NameDef(LexState) *ls, NameDef(expdesc) *v, int limit) {
  NameDef(BinOpr) op;
  NameDef(UnOpr) uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  if (uop != NameDef(OPR_NOUNOPR)) {
    int line = ls->linenumber;
    NameDef(luaX_next)(ls);
    subexpr(ls, v, UNARY_PRIORITY);
    NameDef(luaK_prefix)(ls->fs, uop, v, line);
  }
  else simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  while (op != NameDef(OPR_NOBINOPR) && priority[op].left > limit) {
    NameDef(expdesc) v2;
    NameDef(BinOpr) nextop;
    int line = ls->linenumber;
    NameDef(luaX_next)(ls);
    NameDef(luaK_infix)(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].right);
    NameDef(luaK_posfix)(ls->fs, op, v, &v2, line);
    op = nextop;
  }
  leavelevel(ls);
  return op;  /* return first untreated operator */
}


static void expr (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


static void block (NameDef(LexState) *ls) {
  /* block -> statlist */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(BlockCnt) bl;
  enterblock(fs, &bl, 0);
  statlist(ls);
  leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** assignment
*/
struct NameDef(LHS_assign) {
  struct NameDef(LHS_assign) *prev;
  NameDef(expdesc) v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
static void check_conflict (NameDef(LexState) *ls, struct NameDef(LHS_assign) *lh, NameDef(expdesc) *v) {
  NameDef(FuncState) *fs = ls->fs;
  int extra = fs->freereg;  /* eventual position to save local variable */
  int conflict = 0;
  for (; lh; lh = lh->prev) {  /* check all previous assignments */
    if (lh->v.k == NameDef(VINDEXED)) {  /* assigning to a table? */
      /* table is the upvalue/local being assigned now? */
      if (lh->v.u.ind.vt == v->k && lh->v.u.ind.t == v->u.info) {
        conflict = 1;
        lh->v.u.ind.vt = NameDef(VLOCAL);
        lh->v.u.ind.t = extra;  /* previous assignment will use safe copy */
      }
      /* index is the local being assigned? (index cannot be upvalue) */
      if (v->k == NameDef(VLOCAL) && lh->v.u.ind.idx == v->u.info) {
        conflict = 1;
        lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
      }
    }
  }
  if (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    NameDef(OpCode) op = (v->k == NameDef(VLOCAL)) ? NameDef(OP_MOVE) : NameDef(OP_GETUPVAL);
    NameDef(luaK_codeABC)(fs, op, extra, v->u.info, 0);
    NameDef(luaK_reserveregs)(fs, 1);
  }
}


static void assignment (NameDef(LexState) *ls, struct NameDef(LHS_assign) *lh, int nvars) {
  NameDef(expdesc) e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  if (testnext(ls, ',')) {  /* assignment -> ',' suffixedexp assignment */
    struct NameDef(LHS_assign) nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    if (nv.v.k != NameDef(VINDEXED))
      check_conflict(ls, lh, &nv.v);
    checklimit(ls->fs, nvars + ls->L->nCcalls, LUAI_MAXCCALLS,
                    "C levels");
    assignment(ls, &nv, nvars+1);
  }
  else {  /* assignment -> '=' explist */
    int nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    if (nexps != nvars) {
      adjust_assign(ls, nvars, nexps, &e);
      if (nexps > nvars)
        ls->fs->freereg -= nexps - nvars;  /* remove extra values */
    }
    else {
      NameDef(luaK_setoneret)(ls->fs, &e);  /* close last expression */
      NameDef(luaK_storevar)(ls->fs, &lh->v, &e);
      return;  /* avoid default */
    }
  }
  init_exp(&e, NameDef(VNONRELOC), ls->fs->freereg-1);  /* default assignment */
  NameDef(luaK_storevar)(ls->fs, &lh->v, &e);
}


static int cond (NameDef(LexState) *ls) {
  /* cond -> exp */
  NameDef(expdesc) v;
  expr(ls, &v);  /* read condition */
  if (v.k == NameDef(VNIL)) v.k = NameDef(VFALSE);  /* 'falses' are all equal here */
  NameDef(luaK_goiftrue)(ls->fs, &v);
  return v.f;
}


static void gotostat (NameDef(LexState) *ls, int pc) {
  int line = ls->linenumber;
  NameDef(TString) *label;
  int g;
  if (testnext(ls, NameDef(TK_GOTO)))
    label = str_checkname(ls);
  else {
    NameDef(luaX_next)(ls);  /* skip break */
    label = NameDef(luaS_new)(ls->L, "break");
  }
  g = newlabelentry(ls, &ls->dyd->gt, label, line, pc);
  findlabel(ls, g);  /* close it if label already defined */
}


/* check for repeated labels on the same block */
static void checkrepeated (NameDef(FuncState) *fs, NameDef(Labellist) *ll, NameDef(TString) *label) {
  int i;
  for (i = fs->bl->firstlabel; i < ll->n; i++) {
    if (eqstr(label, ll->arr[i].name)) {
      const char *msg = NameDef(luaO_pushfstring)(fs->ls->L,
                          "label '%s' already defined on line %d",
                          getstr(label), ll->arr[i].line);
      semerror(fs->ls, msg);
    }
  }
}


/* skip no-op statements */
static void skipnoopstat (NameDef(LexState) *ls) {
  while (ls->t.token == ';' || ls->t.token == NameDef(TK_DBCOLON))
    statement(ls);
}


static void labelstat (NameDef(LexState) *ls, NameDef(TString) *label, int line) {
  /* label -> '::' NAME '::' */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(Labellist) *ll = &ls->dyd->label;
  int l;  /* index of new label being created */
  checkrepeated(fs, ll, label);  /* check for repeated labels */
  checknext(ls, NameDef(TK_DBCOLON));  /* skip double colon */
  /* create new entry for this label */
  l = newlabelentry(ls, ll, label, line, NameDef(luaK_getlabel)(fs));
  skipnoopstat(ls);  /* skip other no-op statements */
  if (block_follow(ls, 0)) {  /* label is last no-op statement in the block? */
    /* assume that locals are already out of scope */
    ll->arr[l].nactvar = fs->bl->nactvar;
  }
  findgotos(ls, &ll->arr[l]);
}


static void whilestat (NameDef(LexState) *ls, int line) {
  /* whilestat -> WHILE cond DO block END */
  NameDef(FuncState) *fs = ls->fs;
  int whileinit;
  int condexit;
  NameDef(BlockCnt) bl;
  NameDef(luaX_next)(ls);  /* skip WHILE */
  whileinit = NameDef(luaK_getlabel)(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, NameDef(TK_DO));
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, NameDef(TK_END), NameDef(TK_WHILE), line);
  leaveblock(fs);
  NameDef(luaK_patchtohere)(fs, condexit);  /* false conditions finish the loop */
}


static void repeatstat (NameDef(LexState) *ls, int line) {
  /* repeatstat -> REPEAT block UNTIL cond */
  int condexit;
  NameDef(FuncState) *fs = ls->fs;
  int repeat_init = NameDef(luaK_getlabel)(fs);
  NameDef(BlockCnt) bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  NameDef(luaX_next)(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, NameDef(TK_UNTIL), NameDef(TK_REPEAT), line);
  condexit = cond(ls);  /* read condition (inside scope block) */
  if (bl2.upval)  /* upvalues? */
    NameDef(luaK_patchclose)(fs, condexit, bl2.nactvar);
  leaveblock(fs);  /* finish scope */
  NameDef(luaK_patchlist)(fs, condexit, repeat_init);  /* close the loop */
  leaveblock(fs);  /* finish loop */
}


static int exp1 (NameDef(LexState) *ls) {
  NameDef(expdesc) e;
  int reg;
  expr(ls, &e);
  NameDef(luaK_exp2nextreg)(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
  reg = e.u.info;
  return reg;
}


static void forbody (NameDef(LexState) *ls, int base, int line, int nvars, int isnum) {
  /* forbody -> DO block */
  NameDef(BlockCnt) bl;
  NameDef(FuncState) *fs = ls->fs;
  int prep, endfor;
  adjustlocalvars(ls, 3);  /* control variables */
  checknext(ls, NameDef(TK_DO));
  prep = isnum ? luaK_codeAsBx(fs, NameDef(OP_FORPREP), base, NO_JUMP) : NameDef(luaK_jump)(fs);
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  NameDef(luaK_reserveregs)(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  NameDef(luaK_patchtohere)(fs, prep);
  if (isnum)  /* numeric for? */
    endfor = luaK_codeAsBx(fs, NameDef(OP_FORLOOP), base, NO_JUMP);
  else {  /* generic for */
    NameDef(luaK_codeABC)(fs, NameDef(OP_TFORCALL), base, 0, nvars);
    NameDef(luaK_fixline)(fs, line);
    endfor = luaK_codeAsBx(fs, NameDef(OP_TFORLOOP), base + 2, NO_JUMP);
  }
  NameDef(luaK_patchlist)(fs, endfor, prep + 1);
  NameDef(luaK_fixline)(fs, line);
}


static void fornum (NameDef(LexState) *ls, NameDef(TString) *varname, int line) {
  /* fornum -> NAME = exp1,exp1[,exp1] forbody */
  NameDef(FuncState) *fs = ls->fs;
  int base = fs->freereg;
  new_localvarliteral(ls, "(for index)");
  new_localvarliteral(ls, "(for limit)");
  new_localvarliteral(ls, "(for step)");
  new_localvar(ls, varname);
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  if (testnext(ls, ','))
    exp1(ls);  /* optional step */
  else {  /* default step = 1 */
    NameDef(luaK_codek)(fs, fs->freereg, NameDef(luaK_intK)(fs, 1));
    NameDef(luaK_reserveregs)(fs, 1);
  }
  forbody(ls, base, line, 1, 1);
}


static void forlist (NameDef(LexState) *ls, NameDef(TString) *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(expdesc) e;
  int nvars = 4;  /* gen, state, control, plus at least one declared var */
  int line;
  int base = fs->freereg;
  /* create control variables */
  new_localvarliteral(ls, "(for generator)");
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for control)");
  /* create declared variables */
  new_localvar(ls, indexname);
  while (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, NameDef(TK_IN));
  line = ls->linenumber;
  adjust_assign(ls, 3, explist(ls, &e), &e);
  NameDef(luaK_checkstack)(fs, 3);  /* extra space to call generator */
  forbody(ls, base, line, nvars - 3, 0);
}


static void forstat (NameDef(LexState) *ls, int line) {
  /* forstat -> FOR (fornum | forlist) END */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(TString) *varname;
  NameDef(BlockCnt) bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  NameDef(luaX_next)(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  switch (ls->t.token) {
    case '=': fornum(ls, varname, line); break;
    case ',': case NameDef(TK_IN): forlist(ls, varname); break;
    default: NameDef(luaX_syntaxerror)(ls, "'=' or 'in' expected");
  }
  check_match(ls, NameDef(TK_END), NameDef(TK_FOR), line);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


static void test_then_block (NameDef(LexState) *ls, int *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  NameDef(BlockCnt) bl;
  NameDef(FuncState) *fs = ls->fs;
  NameDef(expdesc) v;
  int jf;  /* instruction to skip 'then' code (if condition is false) */
  NameDef(luaX_next)(ls);  /* skip IF or ELSEIF */
  expr(ls, &v);  /* read condition */
  checknext(ls, NameDef(TK_THEN));
  if (ls->t.token == NameDef(TK_GOTO) || ls->t.token == NameDef(TK_BREAK)) {
    NameDef(luaK_goiffalse)(ls->fs, &v);  /* will jump to label if condition is true */
    enterblock(fs, &bl, 0);  /* must enter block before 'goto' */
    gotostat(ls, v.t);  /* handle goto/break */
    skipnoopstat(ls);  /* skip other no-op statements */
    if (block_follow(ls, 0)) {  /* 'goto' is the entire block? */
      leaveblock(fs);
      return;  /* and that is it */
    }
    else  /* must skip over 'then' part if condition is false */
      jf = NameDef(luaK_jump)(fs);
  }
  else {  /* regular case (not goto/break) */
    NameDef(luaK_goiftrue)(ls->fs, &v);  /* skip over block if condition is false */
    enterblock(fs, &bl, 0);
    jf = v.f;
  }
  statlist(ls);  /* 'then' part */
  leaveblock(fs);
  if (ls->t.token == NameDef(TK_ELSE) ||
      ls->t.token == NameDef(TK_ELSEIF))  /* followed by 'else'/'elseif'? */
    NameDef(luaK_concat)(fs, escapelist, NameDef(luaK_jump)(fs));  /* must jump over it */
  NameDef(luaK_patchtohere)(fs, jf);
}


static void ifstat (NameDef(LexState) *ls, int line) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  NameDef(FuncState) *fs = ls->fs;
  int escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  while (ls->t.token == NameDef(TK_ELSEIF))
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  if (testnext(ls, NameDef(TK_ELSE)))
    block(ls);  /* 'else' part */
  check_match(ls, NameDef(TK_END), NameDef(TK_IF), line);
  NameDef(luaK_patchtohere)(fs, escapelist);  /* patch escape list to 'if' end */
}


static void localfunc (NameDef(LexState) *ls) {
  NameDef(expdesc) b;
  NameDef(FuncState) *fs = ls->fs;
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  getlocvar(fs, b.u.info)->startpc = fs->pc;
}


static void localstat (NameDef(LexState) *ls) {
  /* stat -> LOCAL NAME {',' NAME} ['=' explist] */
  int nvars = 0;
  int nexps;
  NameDef(expdesc) e;
  do {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  } while (testnext(ls, ','));
  if (testnext(ls, '='))
    nexps = explist(ls, &e);
  else {
    e.k = NameDef(VVOID);
    nexps = 0;
  }
  adjust_assign(ls, nvars, nexps, &e);
  adjustlocalvars(ls, nvars);
}


static int funcname (NameDef(LexState) *ls, NameDef(expdesc) *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  int ismethod = 0;
  singlevar(ls, v);
  while (ls->t.token == '.')
    fieldsel(ls, v);
  if (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  return ismethod;
}


static void funcstat (NameDef(LexState) *ls, int line) {
  /* funcstat -> FUNCTION funcname body */
  int ismethod;
  NameDef(expdesc) v, b;
  NameDef(luaX_next)(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  body(ls, &b, ismethod, line);
  NameDef(luaK_storevar)(ls->fs, &v, &b);
  NameDef(luaK_fixline)(ls->fs, line);  /* definition "happens" in the first line */
}


static void exprstat (NameDef(LexState) *ls) {
  /* stat -> func | assignment */
  NameDef(FuncState) *fs = ls->fs;
  struct NameDef(LHS_assign) v;
  suffixedexp(ls, &v.v);
  if (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = NULL;
    assignment(ls, &v, 1);
  }
  else {  /* stat -> func */
    check_condition(ls, v.v.k == NameDef(VCALL), "syntax error");
    SETARG_C(getinstruction(fs, &v.v), 1);  /* call statement uses no results */
  }
}


static void retstat (NameDef(LexState) *ls) {
  /* stat -> RETURN [explist] [';'] */
  NameDef(FuncState) *fs = ls->fs;
  NameDef(expdesc) e;
  int first, nret;  /* registers with returned values */
  if (block_follow(ls, 1) || ls->t.token == ';')
    first = nret = 0;  /* return no values */
  else {
    nret = explist(ls, &e);  /* optional return values */
    if (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      if (e.k == NameDef(VCALL) && nret == 1) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), NameDef(OP_TAILCALL));
        lua_assert(GETARG_A(getinstruction(fs,&e)) == fs->nactvar);
      }
      first = fs->nactvar;
      nret = LUA_MULTRET;  /* return all values */
    }
    else {
      if (nret == 1)  /* only one single value? */
        first = NameDef(luaK_exp2anyreg)(fs, &e);
      else {
        NameDef(luaK_exp2nextreg)(fs, &e);  /* values must go to the stack */
        first = fs->nactvar;  /* return all active values */
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  NameDef(luaK_ret)(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


static void statement (NameDef(LexState) *ls) {
  int line = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  switch (ls->t.token) {
    case ';': {  /* stat -> ';' (empty statement) */
      NameDef(luaX_next)(ls);  /* skip ';' */
      break;
    }
    case NameDef(TK_IF): {  /* stat -> ifstat */
      ifstat(ls, line);
      break;
    }
    case NameDef(TK_WHILE): {  /* stat -> whilestat */
      whilestat(ls, line);
      break;
    }
    case NameDef(TK_DO): {  /* stat -> DO block END */
      NameDef(luaX_next)(ls);  /* skip DO */
      block(ls);
      check_match(ls, NameDef(TK_END), NameDef(TK_DO), line);
      break;
    }
    case NameDef(TK_FOR): {  /* stat -> forstat */
      forstat(ls, line);
      break;
    }
    case NameDef(TK_REPEAT): {  /* stat -> repeatstat */
      repeatstat(ls, line);
      break;
    }
    case NameDef(TK_FUNCTION): {  /* stat -> funcstat */
      funcstat(ls, line);
      break;
    }
    case NameDef(TK_LOCAL): {  /* stat -> localstat */
      NameDef(luaX_next)(ls);  /* skip LOCAL */
      if (testnext(ls, NameDef(TK_FUNCTION)))  /* local function? */
        localfunc(ls);
      else
        localstat(ls);
      break;
    }
    case NameDef(TK_DBCOLON): {  /* stat -> label */
      NameDef(luaX_next)(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), line);
      break;
    }
    case NameDef(TK_RETURN): {  /* stat -> retstat */
      NameDef(luaX_next)(ls);  /* skip RETURN */
      retstat(ls);
      break;
    }
    case NameDef(TK_BREAK):   /* stat -> breakstat */
    case NameDef(TK_GOTO): {  /* stat -> 'goto' NAME */
      gotostat(ls, NameDef(luaK_jump)(ls->fs));
      break;
    }
    default: {  /* stat -> func | assignment */
      exprstat(ls);
      break;
    }
  }
  lua_assert(ls->fs->f->maxstacksize >= ls->fs->freereg &&
             ls->fs->freereg >= ls->fs->nactvar);
  ls->fs->freereg = ls->fs->nactvar;  /* free registers */
  leavelevel(ls);
}

/* }====================================================================== */


/*
** compiles the main function, which is a regular vararg function with an
** upvalue named LUA_ENV
*/
static void mainfunc (NameDef(LexState) *ls, NameDef(FuncState) *fs) {
  NameDef(BlockCnt) bl;
  NameDef(expdesc) v;
  open_func(ls, fs, &bl);
  fs->f->is_vararg = 2;  /* main function is always declared vararg */
  init_exp(&v, NameDef(VLOCAL), 0);  /* create and... */
  newupvalue(fs, ls->envn, &v);  /* ...set environment upvalue */
  NameDef(luaX_next)(ls);  /* read first token */
  statlist(ls);  /* parse main body */
  check(ls, NameDef(TK_EOS));
  close_func(ls);
}


NameDef(LClosure) *NameDef(luaY_parser) (NameDef(lua_State) *L, NameDef(ZIO) *z, NameDef(Mbuffer) *buff,
                       NameDef(Dyndata) *dyd, const char *name, int firstchar) {
  NameDef(LexState) lexstate;
  NameDef(FuncState) funcstate;
  NameDef(LClosure) *cl = NameDef(luaF_newLclosure)(L, 1);  /* create main closure */
  setclLvalue(L, L->top, cl);  /* anchor it (to avoid being collected) */
  NameDef(luaD_inctop)(L);
  lexstate.h = NameDef(luaH_new)(L);  /* create table for scanner */
  sethvalue(L, L->top, lexstate.h);  /* anchor it */
  NameDef(luaD_inctop)(L);
  funcstate.f = cl->p = NameDef(luaF_newproto)(L);
  funcstate.f->source = NameDef(luaS_new)(L, name);  /* create and anchor TString */
  lua_assert(iswhite(funcstate.f));  /* do not need barrier here */
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  NameDef(luaX_setinput)(L, &lexstate, z, funcstate.f->source, firstchar);
  mainfunc(&lexstate, &funcstate);
  lua_assert(!funcstate.prev && funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  L->top--;  /* remove scanner's table */
  return cl;  /* closure is on the stack, too */
}

