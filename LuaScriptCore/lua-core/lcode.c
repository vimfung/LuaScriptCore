/*
** $Id: lcode.c,v 2.101 2015/04/29 18:24:11 roberto Exp $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#include "LuaDefine.h"

#include "lprefix.h"


#include <math.h>
#include <stdlib.h>

#include "lua.h"

#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "llex.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstring.h"
#include "ltable.h"
#include "lvm.h"


/* Maximum number of registers in a Lua function (must fit in 8 bits) */
#define MAXREGS		255


#define hasjumps(e)	((e)->t != (e)->f)


static int tonumeral(NameDef(expdesc) *e, NameDef(TValue) *v) {
  if (e->t != NO_JUMP || e->f != NO_JUMP)
    return 0;  /* not a numeral */
  switch (e->k) {
    case NameDef(VKINT):
      if (v) setivalue(v, e->u.ival);
      return 1;
    case NameDef(VKFLT):
      if (v) setfltvalue(v, e->u.nval);
      return 1;
    default: return 0;
  }
}


void NameDef(luaK_nil) (NameDef(FuncState) *fs, int from, int n) {
  NameDef(Instruction) *previous;
  int l = from + n - 1;  /* last register to set nil */
  if (fs->pc > fs->lasttarget) {  /* no jumps to current position? */
    previous = &fs->f->code[fs->pc-1];
    if (GET_OPCODE(*previous) == NameDef(OP_LOADNIL)) {
      int pfrom = GETARG_A(*previous);
      int pl = pfrom + GETARG_B(*previous);
      if ((pfrom <= from && from <= pl + 1) ||
          (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
        if (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
        if (pl > l) l = pl;  /* l = max(l, pl) */
        SETARG_A(*previous, from);
        SETARG_B(*previous, l - from);
        return;
      }
    }  /* else go through */
  }
  NameDef(luaK_codeABC)(fs, NameDef(OP_LOADNIL), from, n - 1, 0);  /* else no optimization */
}


int NameDef(luaK_jump) (NameDef(FuncState) *fs) {
  int jpc = fs->jpc;  /* save list of jumps to here */
  int j;
  fs->jpc = NO_JUMP;
  j = luaK_codeAsBx(fs, NameDef(OP_JMP), 0, NO_JUMP);
  NameDef(luaK_concat)(fs, &j, jpc);  /* keep them on hold */
  return j;
}


void NameDef(luaK_ret) (NameDef(FuncState) *fs, int first, int nret) {
  NameDef(luaK_codeABC)(fs, NameDef(OP_RETURN), first, nret+1, 0);
}


static int condjump (NameDef(FuncState) *fs, NameDef(OpCode) op, int A, int B, int C) {
  NameDef(luaK_codeABC)(fs, op, A, B, C);
  return NameDef(luaK_jump)(fs);
}


static void fixjump (NameDef(FuncState) *fs, int pc, int dest) {
  NameDef(Instruction) *jmp = &fs->f->code[pc];
  int offset = dest-(pc+1);
  lua_assert(dest != NO_JUMP);
  if (abs(offset) > MAXARG_sBx)
    NameDef(luaX_syntaxerror)(fs->ls, "control structure too long");
  SETARG_sBx(*jmp, offset);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
int NameDef(luaK_getlabel) (NameDef(FuncState) *fs) {
  fs->lasttarget = fs->pc;
  return fs->pc;
}


static int getjump (NameDef(FuncState) *fs, int pc) {
  int offset = GETARG_sBx(fs->f->code[pc]);
  if (offset == NO_JUMP)  /* point to itself represents end of list */
    return NO_JUMP;  /* end of list */
  else
    return (pc+1)+offset;  /* turn offset into absolute position */
}


static NameDef(Instruction) *getjumpcontrol (NameDef(FuncState) *fs, int pc) {
  NameDef(Instruction) *pi = &fs->f->code[pc];
  if (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    return pi-1;
  else
    return pi;
}


/*
** check whether list has any jump that do not produce a value
** (or produce an inverted value)
*/
static int need_value (NameDef(FuncState) *fs, int list) {
  for (; list != NO_JUMP; list = getjump(fs, list)) {
    NameDef(Instruction) i = *getjumpcontrol(fs, list);
    if (GET_OPCODE(i) != NameDef(OP_TESTSET)) return 1;
  }
  return 0;  /* not found */
}


static int patchtestreg (NameDef(FuncState) *fs, int node, int reg) {
  NameDef(Instruction) *i = getjumpcontrol(fs, node);
  if (GET_OPCODE(*i) != NameDef(OP_TESTSET))
    return 0;  /* cannot patch other instructions */
  if (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  else  /* no register to put value or register already has the value */
    *i = CREATE_ABC(NameDef(OP_TEST), GETARG_B(*i), 0, GETARG_C(*i));

  return 1;
}


static void removevalues (NameDef(FuncState) *fs, int list) {
  for (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


static void patchlistaux (NameDef(FuncState) *fs, int list, int vtarget, int reg,
                          int dtarget) {
  while (list != NO_JUMP) {
    int next = getjump(fs, list);
    if (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    else
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


static void dischargejpc (NameDef(FuncState) *fs) {
  patchlistaux(fs, fs->jpc, fs->pc, NO_REG, fs->pc);
  fs->jpc = NO_JUMP;
}


void NameDef(luaK_patchlist) (NameDef(FuncState) *fs, int list, int target) {
  if (target == fs->pc)
    NameDef(luaK_patchtohere)(fs, list);
  else {
    lua_assert(target < fs->pc);
    patchlistaux(fs, list, target, NO_REG, target);
  }
}


void NameDef(luaK_patchclose) (NameDef(FuncState) *fs, int list, int level) {
  level++;  /* argument is +1 to reserve 0 as non-op */
  while (list != NO_JUMP) {
    int next = getjump(fs, list);
    lua_assert(GET_OPCODE(fs->f->code[list]) == OP_JMP &&
                (GETARG_A(fs->f->code[list]) == 0 ||
                 GETARG_A(fs->f->code[list]) >= level));
    SETARG_A(fs->f->code[list], level);
    list = next;
  }
}


void NameDef(luaK_patchtohere) (NameDef(FuncState) *fs, int list) {
  NameDef(luaK_getlabel)(fs);
  NameDef(luaK_concat)(fs, &fs->jpc, list);
}


void NameDef(luaK_concat) (NameDef(FuncState) *fs, int *l1, int l2) {
  if (l2 == NO_JUMP) return;
  else if (*l1 == NO_JUMP)
    *l1 = l2;
  else {
    int list = *l1;
    int next;
    while ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);
  }
}


static int luaK_code (NameDef(FuncState) *fs, NameDef(Instruction) i) {
  NameDef(Proto) *f = fs->f;
  dischargejpc(fs);  /* 'pc' will change */
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, NameDef(Instruction),
                  MAX_INT, "opcodes");
  f->code[fs->pc] = i;
  /* save corresponding line information */
  luaM_growvector(fs->ls->L, f->lineinfo, fs->pc, f->sizelineinfo, int,
                  MAX_INT, "opcodes");
  f->lineinfo[fs->pc] = fs->ls->lastline;
  return fs->pc++;
}


int NameDef(luaK_codeABC) (NameDef(FuncState) *fs, NameDef(OpCode) o, int a, int b, int c) {
  lua_assert(getOpMode(o) == NameDef(iABC));
  lua_assert(getBMode(o) != OpArgN || b == 0);
  lua_assert(getCMode(o) != OpArgN || c == 0);
  lua_assert(a <= MAXARG_A && b <= MAXARG_B && c <= MAXARG_C);
  return luaK_code(fs, CREATE_ABC(o, a, b, c));
}


int NameDef(luaK_codeABx) (NameDef(FuncState) *fs, NameDef(OpCode) o, int a, unsigned int bc) {
  lua_assert(getOpMode(o) == NameDef(iABx) || getOpMode(o) == NameDef(iAsBx));
  lua_assert(getCMode(o) == OpArgN);
  lua_assert(a <= MAXARG_A && bc <= MAXARG_Bx);
  return luaK_code(fs, CREATE_ABx(o, a, bc));
}


static int codeextraarg (NameDef(FuncState) *fs, int a) {
  lua_assert(a <= MAXARG_Ax);
  return luaK_code(fs, CREATE_Ax(NameDef(OP_EXTRAARG), a));
}


int NameDef(luaK_codek) (NameDef(FuncState) *fs, int reg, int k) {
  if (k <= MAXARG_Bx)
    return NameDef(luaK_codeABx)(fs, NameDef(OP_LOADK), reg, k);
  else {
    int p = NameDef(luaK_codeABx)(fs, NameDef(OP_LOADKX), reg, 0);
    codeextraarg(fs, k);
    return p;
  }
}


void NameDef(luaK_checkstack) (NameDef(FuncState) *fs, int n) {
  int newstack = fs->freereg + n;
  if (newstack > fs->f->maxstacksize) {
    if (newstack >= MAXREGS)
      NameDef(luaX_syntaxerror)(fs->ls,
        "function or expression needs too many registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


void NameDef(luaK_reserveregs) (NameDef(FuncState) *fs, int n) {
  NameDef(luaK_checkstack)(fs, n);
  fs->freereg += n;
}


static void freereg (NameDef(FuncState) *fs, int reg) {
  if (!ISK(reg) && reg >= fs->nactvar) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


static void freeexp (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k == NameDef(VNONRELOC))
    freereg(fs, e->u.info);
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants
*/
static int addk (NameDef(FuncState) *fs, NameDef(TValue) *key, NameDef(TValue) *v) {
  NameDef(lua_State) *L = fs->ls->L;
  NameDef(Proto) *f = fs->f;
  NameDef(TValue) *idx = NameDef(luaH_set)(L, fs->ls->h, key);  /* index scanner table */
  int k, oldsize;
  if (ttisinteger(idx)) {  /* is there an index there? */
    k = cast_int(ivalue(idx));
    /* correct value? (warning: must distinguish floats from integers!) */
    if (k < fs->nk && ttype(&f->k[k]) == ttype(v) &&
                      luaV_rawequalobj(&f->k[k], v))
      return k;  /* reuse index */
  }
  /* constant not found; create a new entry */
  oldsize = f->sizek;
  k = fs->nk;
  /* numerical value does not need GC barrier;
     table has no metatable, so it does not need to invalidate cache */
  setivalue(idx, k);
  luaM_growvector(L, f->k, k, f->sizek, NameDef(TValue), MAXARG_Ax, "constants");
  while (oldsize < f->sizek) setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  return k;
}


int NameDef(luaK_stringK) (NameDef(FuncState) *fs, NameDef(TString) *s) {
  NameDef(TValue) o;
  setsvalue(fs->ls->L, &o, s);
  return addk(fs, &o, &o);
}


/*
** Integers use userdata as keys to avoid collision with floats with same
** value; conversion to 'void*' used only for hashing, no "precision"
** problems
*/
int NameDef(luaK_intK) (NameDef(FuncState) *fs, NameDef(lua_Integer) n) {
  NameDef(TValue) k, o;
  setpvalue(&k, cast(void*, cast(size_t, n)));
  setivalue(&o, n);
  return addk(fs, &k, &o);
}


static int luaK_numberK (NameDef(FuncState) *fs, NameDef(lua_Number) r) {
  NameDef(TValue) o;
  setfltvalue(&o, r);
  return addk(fs, &o, &o);
}


static int boolK (NameDef(FuncState) *fs, int b) {
  NameDef(TValue) o;
  setbvalue(&o, b);
  return addk(fs, &o, &o);
}


static int nilK (NameDef(FuncState) *fs) {
  NameDef(TValue) k, v;
  setnilvalue(&v);
  /* cannot use nil as key; instead use table itself to represent nil */
  sethvalue(fs->ls->L, &k, fs->ls->h);
  return addk(fs, &k, &v);
}


void NameDef(luaK_setreturns) (NameDef(FuncState) *fs, NameDef(expdesc) *e, int nresults) {
  if (e->k == NameDef(VCALL)) {  /* expression is an open function call? */
    SETARG_C(getcode(fs, e), nresults+1);
  }
  else if (e->k == NameDef(VVARARG)) {
    SETARG_B(getcode(fs, e), nresults+1);
    SETARG_A(getcode(fs, e), fs->freereg);
    NameDef(luaK_reserveregs)(fs, 1);
  }
}


void NameDef(luaK_setoneret) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k == NameDef(VCALL)) {  /* expression is an open function call? */
    e->k = NameDef(VNONRELOC);
    e->u.info = GETARG_A(getcode(fs, e));
  }
  else if (e->k == NameDef(VVARARG)) {
    SETARG_B(getcode(fs, e), 2);
    e->k = NameDef(VRELOCABLE);  /* can relocate its simple result */
  }
}


void NameDef(luaK_dischargevars) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  switch (e->k) {
    case NameDef(VLOCAL): {
      e->k = NameDef(VNONRELOC);
      break;
    }
    case NameDef(VUPVAL): {
      e->u.info = NameDef(luaK_codeABC)(fs, NameDef(OP_GETUPVAL), 0, e->u.info, 0);
      e->k = NameDef(VRELOCABLE);
      break;
    }
    case NameDef(VINDEXED): {
      NameDef(OpCode) op = NameDef(OP_GETTABUP);  /* assume 't' is in an upvalue */
      freereg(fs, e->u.ind.idx);
      if (e->u.ind.vt == NameDef(VLOCAL)) {  /* 't' is in a register? */
        freereg(fs, e->u.ind.t);
        op = NameDef(OP_GETTABLE);
      }
      e->u.info = NameDef(luaK_codeABC)(fs, op, 0, e->u.ind.t, e->u.ind.idx);
      e->k = NameDef(VRELOCABLE);
      break;
    }
    case NameDef(VVARARG):
    case NameDef(VCALL): {
      NameDef(luaK_setoneret)(fs, e);
      break;
    }
    default: break;  /* there is one value available (somewhere) */
  }
}


static int code_label (NameDef(FuncState) *fs, int A, int b, int jump) {
  NameDef(luaK_getlabel)(fs);  /* those instructions may be jump targets */
  return NameDef(luaK_codeABC)(fs, NameDef(OP_LOADBOOL), A, b, jump);
}


static void discharge2reg (NameDef(FuncState) *fs, NameDef(expdesc) *e, int reg) {
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VNIL): {
      NameDef(luaK_nil)(fs, reg, 1);
      break;
    }
    case NameDef(VFALSE): case NameDef(VTRUE): {
      NameDef(luaK_codeABC)(fs, NameDef(OP_LOADBOOL), reg, e->k == NameDef(VTRUE), 0);
      break;
    }
    case NameDef(VK): {
      NameDef(luaK_codek)(fs, reg, e->u.info);
      break;
    }
    case NameDef(VKFLT): {
      NameDef(luaK_codek)(fs, reg, luaK_numberK(fs, e->u.nval));
      break;
    }
    case NameDef(VKINT): {
      NameDef(luaK_codek)(fs, reg, NameDef(luaK_intK)(fs, e->u.ival));
      break;
    }
    case NameDef(VRELOCABLE): {
      NameDef(Instruction) *pc = &getcode(fs, e);
      SETARG_A(*pc, reg);
      break;
    }
    case NameDef(VNONRELOC): {
      if (reg != e->u.info)
        NameDef(luaK_codeABC)(fs, NameDef(OP_MOVE), reg, e->u.info, 0);
      break;
    }
    default: {
      lua_assert(e->k == VVOID || e->k == VJMP);
      return;  /* nothing to do... */
    }
  }
  e->u.info = reg;
  e->k = NameDef(VNONRELOC);
}


static void discharge2anyreg (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k != NameDef(VNONRELOC)) {
    NameDef(luaK_reserveregs)(fs, 1);
    discharge2reg(fs, e, fs->freereg-1);
  }
}


static void exp2reg (NameDef(FuncState) *fs, NameDef(expdesc) *e, int reg) {
  discharge2reg(fs, e, reg);
  if (e->k == NameDef(VJMP))
    NameDef(luaK_concat)(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  if (hasjumps(e)) {
    int final;  /* position after whole expression */
    int p_f = NO_JUMP;  /* position of an eventual LOAD false */
    int p_t = NO_JUMP;  /* position of an eventual LOAD true */
    if (need_value(fs, e->t) || need_value(fs, e->f)) {
      int fj = (e->k == NameDef(VJMP)) ? NO_JUMP : NameDef(luaK_jump)(fs);
      p_f = code_label(fs, reg, 0, 1);
      p_t = code_label(fs, reg, 1, 0);
      NameDef(luaK_patchtohere)(fs, fj);
    }
    final = NameDef(luaK_getlabel)(fs);
    patchlistaux(fs, e->f, final, reg, p_f);
    patchlistaux(fs, e->t, final, reg, p_t);
  }
  e->f = e->t = NO_JUMP;
  e->u.info = reg;
  e->k = NameDef(VNONRELOC);
}


void NameDef(luaK_exp2nextreg) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_dischargevars)(fs, e);
  freeexp(fs, e);
  NameDef(luaK_reserveregs)(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


int NameDef(luaK_exp2anyreg) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_dischargevars)(fs, e);
  if (e->k == NameDef(VNONRELOC)) {
    if (!hasjumps(e)) return e->u.info;  /* exp is already in a register */
    if (e->u.info >= fs->nactvar) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put value on it */
      return e->u.info;
    }
  }
  NameDef(luaK_exp2nextreg)(fs, e);  /* default */
  return e->u.info;
}


void NameDef(luaK_exp2anyregup) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k != NameDef(VUPVAL) || hasjumps(e))
    NameDef(luaK_exp2anyreg)(fs, e);
}


void NameDef(luaK_exp2val) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (hasjumps(e))
    NameDef(luaK_exp2anyreg)(fs, e);
  else
    NameDef(luaK_dischargevars)(fs, e);
}


int NameDef(luaK_exp2RK) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_exp2val)(fs, e);
  switch (e->k) {
    case NameDef(VTRUE):
    case NameDef(VFALSE):
    case NameDef(VNIL): {
      if (fs->nk <= MAXINDEXRK) {  /* constant fits in RK operand? */
        e->u.info = (e->k == NameDef(VNIL)) ? nilK(fs) : boolK(fs, (e->k == NameDef(VTRUE)));
        e->k = NameDef(VK);
        return RKASK(e->u.info);
      }
      else break;
    }
    case NameDef(VKINT): {
      e->u.info = NameDef(luaK_intK)(fs, e->u.ival);
      e->k = NameDef(VK);
      goto vk;
    }
    case NameDef(VKFLT): {
      e->u.info = luaK_numberK(fs, e->u.nval);
      e->k = NameDef(VK);
    }
    /* FALLTHROUGH */
    case NameDef(VK): {
     vk:
      if (e->u.info <= MAXINDEXRK)  /* constant fits in 'argC'? */
        return RKASK(e->u.info);
      else break;
    }
    default: break;
  }
  /* not a constant in the right range: put it in a register */
  return NameDef(luaK_exp2anyreg)(fs, e);
}


void NameDef(luaK_storevar) (NameDef(FuncState) *fs, NameDef(expdesc) *var, NameDef(expdesc) *ex) {
  switch (var->k) {
    case NameDef(VLOCAL): {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.info);
      return;
    }
    case NameDef(VUPVAL): {
      int e = NameDef(luaK_exp2anyreg)(fs, ex);
      NameDef(luaK_codeABC)(fs, NameDef(OP_SETUPVAL), e, var->u.info, 0);
      break;
    }
    case NameDef(VINDEXED): {
      NameDef(OpCode) op = (var->u.ind.vt == NameDef(VLOCAL)) ? NameDef(OP_SETTABLE) : NameDef(OP_SETTABUP);
      int e = NameDef(luaK_exp2RK)(fs, ex);
      NameDef(luaK_codeABC)(fs, op, var->u.ind.t, var->u.ind.idx, e);
      break;
    }
    default: {
      lua_assert(0);  /* invalid var kind to store */
      break;
    }
  }
  freeexp(fs, ex);
}


void NameDef(luaK_self) (NameDef(FuncState) *fs, NameDef(expdesc) *e, NameDef(expdesc) *key) {
  int ereg;
  NameDef(luaK_exp2anyreg)(fs, e);
  ereg = e->u.info;  /* register where 'e' was placed */
  freeexp(fs, e);
  e->u.info = fs->freereg;  /* base register for op_self */
  e->k = NameDef(VNONRELOC);
  NameDef(luaK_reserveregs)(fs, 2);  /* function and 'self' produced by op_self */
  NameDef(luaK_codeABC)(fs, NameDef(OP_SELF), e->u.info, ereg, NameDef(luaK_exp2RK)(fs, key));
  freeexp(fs, key);
}


static void invertjump (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(Instruction) *pc = getjumpcontrol(fs, e->u.info);
  lua_assert(testTMode(GET_OPCODE(*pc)) && GET_OPCODE(*pc) != OP_TESTSET &&
                                           GET_OPCODE(*pc) != OP_TEST);
  SETARG_A(*pc, !(GETARG_A(*pc)));
}


static int jumponcond (NameDef(FuncState) *fs, NameDef(expdesc) *e, int cond) {
  if (e->k == NameDef(VRELOCABLE)) {
    NameDef(Instruction) ie = getcode(fs, e);
    if (GET_OPCODE(ie) == NameDef(OP_NOT)) {
      fs->pc--;  /* remove previous OP_NOT */
      return condjump(fs, NameDef(OP_TEST), GETARG_B(ie), 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  return condjump(fs, NameDef(OP_TESTSET), NO_REG, e->u.info, cond);
}


void NameDef(luaK_goiftrue) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  int pc;  /* pc of last jump */
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VJMP): {
      invertjump(fs, e);
      pc = e->u.info;
      break;
    }
    case NameDef(VK): case NameDef(VKFLT): case NameDef(VKINT): case NameDef(VTRUE): {
      pc = NO_JUMP;  /* always true; do nothing */
      break;
    }
    default: {
      pc = jumponcond(fs, e, 0);
      break;
    }
  }
  NameDef(luaK_concat)(fs, &e->f, pc);  /* insert last jump in 'f' list */
  NameDef(luaK_patchtohere)(fs, e->t);
  e->t = NO_JUMP;
}


void NameDef(luaK_goiffalse) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  int pc;  /* pc of last jump */
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VJMP): {
      pc = e->u.info;
      break;
    }
    case NameDef(VNIL): case NameDef(VFALSE): {
      pc = NO_JUMP;  /* always false; do nothing */
      break;
    }
    default: {
      pc = jumponcond(fs, e, 1);
      break;
    }
  }
  NameDef(luaK_concat)(fs, &e->t, pc);  /* insert last jump in 't' list */
  NameDef(luaK_patchtohere)(fs, e->f);
  e->f = NO_JUMP;
}


static void codenot (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VNIL): case NameDef(VFALSE): {
      e->k = NameDef(VTRUE);
      break;
    }
    case NameDef(VK): case NameDef(VKFLT): case NameDef(VKINT): case NameDef(VTRUE): {
      e->k = NameDef(VFALSE);
      break;
    }
    case NameDef(VJMP): {
      invertjump(fs, e);
      break;
    }
    case NameDef(VRELOCABLE):
    case NameDef(VNONRELOC): {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = NameDef(luaK_codeABC)(fs, NameDef(OP_NOT), 0, e->u.info, 0);
      e->k = NameDef(VRELOCABLE);
      break;
    }
    default: {
      lua_assert(0);  /* cannot happen */
      break;
    }
  }
  /* interchange true and false lists */
  { int temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);
  removevalues(fs, e->t);
}


void NameDef(luaK_indexed) (NameDef(FuncState) *fs, NameDef(expdesc) *t, NameDef(expdesc) *k) {
  lua_assert(!hasjumps(t));
  t->u.ind.t = t->u.info;
  t->u.ind.idx = NameDef(luaK_exp2RK)(fs, k);
  t->u.ind.vt = (t->k == NameDef(VUPVAL)) ? NameDef(VUPVAL)
                                 : check_exp(vkisinreg(t->k), NameDef(VLOCAL));
  t->k = NameDef(VINDEXED);
}


/*
** return false if folding can raise an error
*/
static int validop (int op, NameDef(TValue) *v1, NameDef(TValue) *v2) {
  switch (op) {
    case LUA_OPBAND: case LUA_OPBOR: case LUA_OPBXOR:
    case LUA_OPSHL: case LUA_OPSHR: case LUA_OPBNOT: {  /* conversion errors */
      NameDef(lua_Integer) i;
      return (tointeger(v1, &i) && tointeger(v2, &i));
    }
    case LUA_OPDIV: case LUA_OPIDIV: case LUA_OPMOD:  /* division by 0 */
      return (nvalue(v2) != 0);
    default: return 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful
*/
static int constfolding (NameDef(FuncState) *fs, int op, NameDef(expdesc) *e1, NameDef(expdesc) *e2) {
  NameDef(TValue) v1, v2, res;
  if (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    return 0;  /* non-numeric operands or not safe to fold */
  NameDef(luaO_arith)(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  if (ttisinteger(&res)) {
    e1->k = NameDef(VKINT);
    e1->u.ival = ivalue(&res);
  }
  else {  /* folds neither NaN nor 0.0 (to avoid collapsing with -0.0) */
    NameDef(lua_Number) n = fltvalue(&res);
    if (luai_numisnan(n) || n == 0)
      return 0;
    e1->k = NameDef(VKFLT);
    e1->u.nval = n;
  }
  return 1;
}


/*
** Code for binary and unary expressions that "produce values"
** (arithmetic operations, bitwise operations, concat, length). First
** try to do constant folding (only for numeric [arithmetic and
** bitwise] operations, which is what 'lua_arith' accepts).
** Expression to produce final result will be encoded in 'e1'.
*/
static void codeexpval (NameDef(FuncState) *fs, NameDef(OpCode) op,
                        NameDef(expdesc) *e1, NameDef(expdesc) *e2, int line) {
  lua_assert(op >= NameDef(OP_ADD));
  if (op <= NameDef(OP_BNOT) && constfolding(fs, (op - NameDef(OP_ADD)) + LUA_OPADD, e1, e2))
    return;  /* result has been folded */
  else {
    int o1, o2;
    /* move operands to registers (if needed) */
    if (op == NameDef(OP_UNM) || op == NameDef(OP_BNOT) || op == NameDef(OP_LEN)) {  /* unary op? */
      o2 = 0;  /* no second expression */
      o1 = NameDef(luaK_exp2anyreg)(fs, e1);  /* cannot operate on constants */
    }
    else {  /* regular case (binary operators) */
      o2 = NameDef(luaK_exp2RK)(fs, e2);  /* both operands are "RK" */
      o1 = NameDef(luaK_exp2RK)(fs, e1);
    }
    if (o1 > o2) {  /* free registers in proper order */
      freeexp(fs, e1);
      freeexp(fs, e2);
    }
    else {
      freeexp(fs, e2);
      freeexp(fs, e1);
    }
    e1->u.info = NameDef(luaK_codeABC)(fs, op, 0, o1, o2);  /* generate opcode */
    e1->k = NameDef(VRELOCABLE);  /* all those operations are relocable */
    NameDef(luaK_fixline)(fs, line);
  }
}


static void codecomp (NameDef(FuncState) *fs, NameDef(OpCode) op, int cond, NameDef(expdesc) *e1,
                                                          NameDef(expdesc) *e2) {
  int o1 = NameDef(luaK_exp2RK)(fs, e1);
  int o2 = NameDef(luaK_exp2RK)(fs, e2);
  freeexp(fs, e2);
  freeexp(fs, e1);
  if (cond == 0 && op != NameDef(OP_EQ)) {
    int temp;  /* exchange args to replace by '<' or '<=' */
    temp = o1; o1 = o2; o2 = temp;  /* o1 <==> o2 */
    cond = 1;
  }
  e1->u.info = condjump(fs, op, cond, o1, o2);
  e1->k = NameDef(VJMP);
}


void NameDef(luaK_prefix) (NameDef(FuncState) *fs, NameDef(UnOpr) op, NameDef(expdesc) *e, int line) {
  NameDef(expdesc) e2;
  e2.t = e2.f = NO_JUMP; e2.k = NameDef(VKINT); e2.u.ival = 0;
  switch (op) {
    case NameDef(OPR_MINUS): case NameDef(OPR_BNOT): case NameDef(OPR_LEN): {
      codeexpval(fs, cast(NameDef(OpCode), (op - NameDef(OPR_MINUS)) + NameDef(OP_UNM)), e, &e2, line);
      break;
    }
    case NameDef(OPR_NOT): codenot(fs, e); break;
    default: lua_assert(0);
  }
}


void NameDef(luaK_infix) (NameDef(FuncState) *fs, NameDef(BinOpr) op, NameDef(expdesc) *v) {
  switch (op) {
    case NameDef(OPR_AND): {
      NameDef(luaK_goiftrue)(fs, v);
      break;
    }
    case NameDef(OPR_OR): {
      NameDef(luaK_goiffalse)(fs, v);
      break;
    }
    case NameDef(OPR_CONCAT): {
      NameDef(luaK_exp2nextreg)(fs, v);  /* operand must be on the 'stack' */
      break;
    }
    case NameDef(OPR_ADD): case NameDef(OPR_SUB):
    case NameDef(OPR_MUL): case NameDef(OPR_DIV): case NameDef(OPR_IDIV):
    case NameDef(OPR_MOD): case NameDef(OPR_POW):
    case NameDef(OPR_BAND): case NameDef(OPR_BOR): case NameDef(OPR_BXOR):
    case NameDef(OPR_SHL): case NameDef(OPR_SHR): {
      if (!tonumeral(v, NULL)) NameDef(luaK_exp2RK)(fs, v);
      break;
    }
    default: {
      NameDef(luaK_exp2RK)(fs, v);
      break;
    }
  }
}


void NameDef(luaK_posfix) (NameDef(FuncState) *fs, NameDef(BinOpr) op,
                  NameDef(expdesc) *e1, NameDef(expdesc) *e2, int line) {
  switch (op) {
    case NameDef(OPR_AND): {
      lua_assert(e1->t == NO_JUMP);  /* list must be closed */
      NameDef(luaK_dischargevars)(fs, e2);
      NameDef(luaK_concat)(fs, &e2->f, e1->f);
      *e1 = *e2;
      break;
    }
    case NameDef(OPR_OR): {
      lua_assert(e1->f == NO_JUMP);  /* list must be closed */
      NameDef(luaK_dischargevars)(fs, e2);
      NameDef(luaK_concat)(fs, &e2->t, e1->t);
      *e1 = *e2;
      break;
    }
    case NameDef(OPR_CONCAT): {
      NameDef(luaK_exp2val)(fs, e2);
      if (e2->k == NameDef(VRELOCABLE) && GET_OPCODE(getcode(fs, e2)) == NameDef(OP_CONCAT)) {
        lua_assert(e1->u.info == GETARG_B(getcode(fs, e2))-1);
        freeexp(fs, e1);
        SETARG_B(getcode(fs, e2), e1->u.info);
        e1->k = NameDef(VRELOCABLE); e1->u.info = e2->u.info;
      }
      else {
        NameDef(luaK_exp2nextreg)(fs, e2);  /* operand must be on the 'stack' */
        codeexpval(fs, NameDef(OP_CONCAT), e1, e2, line);
      }
      break;
    }
    case NameDef(OPR_ADD): case NameDef(OPR_SUB): case NameDef(OPR_MUL): case NameDef(OPR_DIV):
    case NameDef(OPR_IDIV): case NameDef(OPR_MOD): case NameDef(OPR_POW):
    case NameDef(OPR_BAND): case NameDef(OPR_BOR): case NameDef(OPR_BXOR):
    case NameDef(OPR_SHL): case NameDef(OPR_SHR): {
      codeexpval(fs, cast(NameDef(OpCode), (op - NameDef(OPR_ADD)) + NameDef(OP_ADD)), e1, e2, line);
      break;
    }
    case NameDef(OPR_EQ): case NameDef(OPR_LT): case NameDef(OPR_LE): {
      codecomp(fs, cast(NameDef(OpCode), (op - NameDef(OPR_EQ)) + NameDef(OP_EQ)), 1, e1, e2);
      break;
    }
    case NameDef(OPR_NE): case NameDef(OPR_GT): case NameDef(OPR_GE): {
      codecomp(fs, cast(NameDef(OpCode), (op - NameDef(OPR_NE)) + NameDef(OP_EQ)), 0, e1, e2);
      break;
    }
    default: lua_assert(0);
  }
}


void NameDef(luaK_fixline) (NameDef(FuncState) *fs, int line) {
  fs->f->lineinfo[fs->pc - 1] = line;
}


void NameDef(luaK_setlist) (NameDef(FuncState) *fs, int base, int nelems, int tostore) {
  int c =  (nelems - 1)/LFIELDS_PER_FLUSH + 1;
  int b = (tostore == LUA_MULTRET) ? 0 : tostore;
  lua_assert(tostore != 0);
  if (c <= MAXARG_C)
    NameDef(luaK_codeABC)(fs, NameDef(OP_SETLIST), base, b, c);
  else if (c <= MAXARG_Ax) {
    NameDef(luaK_codeABC)(fs, NameDef(OP_SETLIST), base, b, 0);
    codeextraarg(fs, c);
  }
  else
    NameDef(luaX_syntaxerror)(fs->ls, "constructor too long");
  fs->freereg = base + 1;  /* free registers with list values */
}

