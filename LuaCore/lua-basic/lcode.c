/*
** $Id: lcode.c,v 2.109 2016/05/13 19:09:21 roberto Exp $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

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


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
static int tonumeral(NameDef(expdesc) *e, NameDef(TValue) *v) {
  if (hasjumps(e))
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


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
void NameDef(luaK_nil) (NameDef(FuncState) *fs, int from, int n) {
  NameDef(Instruction) *previous;
  int l = from + n - 1;  /* last register to set nil */
  if (fs->pc > fs->lasttarget) {  /* no jumps to current position? */
    previous = &fs->f->code[fs->pc-1];
    if (GET_OPCODE(*previous) == NameDef(OP_LOADNIL)) {  /* previous is LOADNIL? */
      int pfrom = GETARG_A(*previous);  /* get previous range */
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


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/ 
static int getjump (NameDef(FuncState) *fs, int pc) {
  int offset = GETARG_sBx(fs->f->code[pc]);
  if (offset == NO_JUMP)  /* point to itself represents end of list */
    return NO_JUMP;  /* end of list */
  else
    return (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
static void fixjump (NameDef(FuncState) *fs, int pc, int dest) {
  NameDef(Instruction) *jmp = &fs->f->code[pc];
  int offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  if (abs(offset) > MAXARG_sBx)
    NameDef(luaX_syntaxerror)(fs->ls, "control structure too long");
  SETARG_sBx(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
void NameDef(luaK_concat) (NameDef(FuncState) *fs, int *l1, int l2) {
  if (l2 == NO_JUMP) return;  /* nothing to concatenate? */
  else if (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  else {
    int list = *l1;
    int next;
    while ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump'). If there are jumps to
** this position (kept in 'jpc'), link them all together so that
** 'patchlistaux' will fix all them directly to the final destination.
*/
int NameDef(luaK_jump) (NameDef(FuncState) *fs) {
  int jpc = fs->jpc;  /* save list of jumps to here */
  int j;
  fs->jpc = NO_JUMP;  /* no more jumps to here */
  j = luaK_codeAsBx(fs, NameDef(OP_JMP), 0, NO_JUMP);
  NameDef(luaK_concat)(fs, &j, jpc);  /* keep them on hold */
  return j;
}


/*
** Code a 'return' instruction
*/
void NameDef(luaK_ret) (NameDef(FuncState) *fs, int first, int nret) {
  NameDef(luaK_codeABC)(fs, NameDef(OP_RETURN), first, nret+1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
static int condjump (NameDef(FuncState) *fs, NameDef(OpCode) op, int A, int B, int C) {
  NameDef(luaK_codeABC)(fs, op, A, B, C);
  return NameDef(luaK_jump)(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
int NameDef(luaK_getlabel) (NameDef(FuncState) *fs) {
  fs->lasttarget = fs->pc;
  return fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
static NameDef(Instruction) *getjumpcontrol (NameDef(FuncState) *fs, int pc) {
  NameDef(Instruction) *pi = &fs->f->code[pc];
  if (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    return pi-1;
  else
    return pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
static int patchtestreg (NameDef(FuncState) *fs, int node, int reg) {
  NameDef(Instruction) *i = getjumpcontrol(fs, node);
  if (GET_OPCODE(*i) != NameDef(OP_TESTSET))
    return 0;  /* cannot patch other instructions */
  if (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  else {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABC(NameDef(OP_TEST), GETARG_B(*i), 0, GETARG_C(*i));
  }
  return 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
static void removevalues (NameDef(FuncState) *fs, int list) {
  for (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
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


/*
** Ensure all pending jumps to current position are fixed (jumping
** to current position with no values) and reset list of pending
** jumps
*/
static void dischargejpc (NameDef(FuncState) *fs) {
  patchlistaux(fs, fs->jpc, fs->pc, NO_REG, fs->pc);
  fs->jpc = NO_JUMP;
}


/*
** Add elements in 'list' to list of pending jumps to "here"
** (current position)
*/
void NameDef(luaK_patchtohere) (NameDef(FuncState) *fs, int list) {
  NameDef(luaK_getlabel)(fs);  /* mark "here" as a jump target */
  NameDef(luaK_concat)(fs, &fs->jpc, list);
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
void NameDef(luaK_patchlist) (NameDef(FuncState) *fs, int list, int target) {
  if (target == fs->pc)  /* 'target' is current position? */
    NameDef(luaK_patchtohere)(fs, list);  /* add list to pending jumps */
  else {
    lua_assert(target < fs->pc);
    patchlistaux(fs, list, target, NO_REG, target);
  }
}


/*
** Path all jumps in 'list' to close upvalues up to given 'level'
** (The assertion checks that jumps either were closing nothing
** or were closing higher levels, from inner blocks.)
*/
void NameDef(luaK_patchclose) (NameDef(FuncState) *fs, int list, int level) {
  level++;  /* argument is +1 to reserve 0 as non-op */
  for (; list != NO_JUMP; list = getjump(fs, list)) {
    lua_assert(GET_OPCODE(fs->f->code[list]) == OP_JMP &&
                (GETARG_A(fs->f->code[list]) == 0 ||
                 GETARG_A(fs->f->code[list]) >= level));
    SETARG_A(fs->f->code[list], level);
  }
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
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


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
int NameDef(luaK_codeABC) (NameDef(FuncState) *fs, NameDef(OpCode) o, int a, int b, int c) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(getBMode(o) != OpArgN || b == 0);
  lua_assert(getCMode(o) != OpArgN || c == 0);
  lua_assert(a <= MAXARG_A && b <= MAXARG_B && c <= MAXARG_C);
  return luaK_code(fs, CREATE_ABC(o, a, b, c));
}


/*
** Format and emit an 'iABx' instruction.
*/
int NameDef(luaK_codeABx) (NameDef(FuncState) *fs, NameDef(OpCode) o, int a, unsigned int bc) {
  lua_assert(getOpMode(o) == iABx || getOpMode(o) == iAsBx);
  lua_assert(getCMode(o) == OpArgN);
  lua_assert(a <= MAXARG_A && bc <= MAXARG_Bx);
  return luaK_code(fs, CREATE_ABx(o, a, bc));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
static int codeextraarg (NameDef(FuncState) *fs, int a) {
  lua_assert(a <= MAXARG_Ax);
  return luaK_code(fs, CREATE_Ax(NameDef(OP_EXTRAARG), a));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
int NameDef(luaK_codek) (NameDef(FuncState) *fs, int reg, int k) {
  if (k <= MAXARG_Bx)
    return NameDef(luaK_codeABx)(fs, NameDef(OP_LOADK), reg, k);
  else {
    int p = NameDef(luaK_codeABx)(fs, NameDef(OP_LOADKX), reg, 0);
    codeextraarg(fs, k);
    return p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
void NameDef(luaK_checkstack) (NameDef(FuncState) *fs, int n) {
  int newstack = fs->freereg + n;
  if (newstack > fs->f->maxstacksize) {
    if (newstack >= MAXREGS)
      NameDef(luaX_syntaxerror)(fs->ls,
        "function or expression needs too many registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
void NameDef(luaK_reserveregs) (NameDef(FuncState) *fs, int n) {
  NameDef(luaK_checkstack)(fs, n);
  fs->freereg += n;
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
static void freereg (NameDef(FuncState) *fs, int reg) {
  if (!ISK(reg) && reg >= fs->nactvar) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
static void freeexp (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k == NameDef(VNONRELOC))
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
static void freeexps (NameDef(FuncState) *fs, NameDef(expdesc) *e1, NameDef(expdesc) *e2) {
  int r1 = (e1->k == NameDef(VNONRELOC)) ? e1->u.info : -1;
  int r2 = (e2->k == NameDef(VNONRELOC)) ? e2->u.info : -1;
  if (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  else {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
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


/*
** Add a string to list of constants and return its index.
*/
int NameDef(luaK_stringK) (NameDef(FuncState) *fs, NameDef(TString) *s) {
  NameDef(TValue) o;
  setsvalue(fs->ls->L, &o, s);
  return addk(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
** Integers use userdata as keys to avoid collision with floats with
** same value; conversion to 'void*' is used only for hashing, so there
** are no "precision" problems.
*/
int NameDef(luaK_intK) (NameDef(FuncState) *fs, NameDef(lua_Integer) n) {
  NameDef(TValue) k, o;
  setpvalue(&k, cast(void*, cast(size_t, n)));
  setivalue(&o, n);
  return addk(fs, &k, &o);
}

/*
** Add a float to list of constants and return its index.
*/
static int luaK_numberK (NameDef(FuncState) *fs, NameDef(lua_Number) r) {
  NameDef(TValue) o;
  setfltvalue(&o, r);
  return addk(fs, &o, &o);  /* use number itself as key */
}


/*
** Add a boolean to list of constants and return its index.
*/
static int boolK (NameDef(FuncState) *fs, int b) {
  NameDef(TValue) o;
  setbvalue(&o, b);
  return addk(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
static int nilK (NameDef(FuncState) *fs) {
  NameDef(TValue) k, v;
  setnilvalue(&v);
  /* cannot use nil as key; instead use table itself to represent nil */
  sethvalue(fs->ls->L, &k, fs->ls->h);
  return addk(fs, &k, &v);
}


/*
** Fix an expression to return the number of results 'nresults'.
** Either 'e' is a multi-ret expression (function call or vararg)
** or 'nresults' is LUA_MULTRET (as any expression can satisfy that).
*/
void NameDef(luaK_setreturns) (NameDef(FuncState) *fs, NameDef(expdesc) *e, int nresults) {
  if (e->k == NameDef(VCALL)) {  /* expression is an open function call? */
    SETARG_C(getinstruction(fs, e), nresults + 1);
  }
  else if (e->k == NameDef(VVARARG)) {
    NameDef(Instruction) *pc = &getinstruction(fs, e);
    SETARG_B(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    NameDef(luaK_reserveregs)(fs, 1);
  }
  else lua_assert(nresults == LUA_MULTRET);
}


/*
** Fix an expression to return one result.
** If expression is not a multi-ret expression (function call or
** vararg), it already returns one result, so nothing needs to be done.
** Function calls become VNONRELOC expressions (as its result comes
** fixed in the base register of the call), while vararg expressions
** become VRELOCABLE (as OP_VARARG puts its results where it wants).
** (Calls are created returning one result, so that does not need
** to be fixed.)
*/
void NameDef(luaK_setoneret) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k == NameDef(VCALL)) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = NameDef(VNONRELOC);  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  else if (e->k == NameDef(VVARARG)) {
    SETARG_B(getinstruction(fs, e), 2);
    e->k = NameDef(VRELOCABLE);  /* can relocate its simple result */
  }
}


/*
** Ensure that expression 'e' is not a variable.
*/
void NameDef(luaK_dischargevars) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  switch (e->k) {
    case NameDef(VLOCAL): {  /* already in a register */
      e->k = NameDef(VNONRELOC);  /* becomes a non-relocatable value */
      break;
    }
    case NameDef(VUPVAL): {  /* move value to some (pending) register */
      e->u.info = NameDef(luaK_codeABC)(fs, NameDef(OP_GETUPVAL), 0, e->u.info, 0);
      e->k = NameDef(VRELOCABLE);
      break;
    }
    case NameDef(VINDEXED): {
      NameDef(OpCode) op;
      freereg(fs, e->u.ind.idx);
      if (e->u.ind.vt == NameDef(VLOCAL)) {  /* is 't' in a register? */
        freereg(fs, e->u.ind.t);
        op = NameDef(OP_GETTABLE);
      }
      else {
        lua_assert(e->u.ind.vt == VUPVAL);
        op = NameDef(OP_GETTABUP);  /* 't' is in an upvalue */
      }
      e->u.info = NameDef(luaK_codeABC)(fs, op, 0, e->u.ind.t, e->u.ind.idx);
      e->k = NameDef(VRELOCABLE);
      break;
    }
    case NameDef(VVARARG): case NameDef(VCALL): {
      NameDef(luaK_setoneret)(fs, e);
      break;
    }
    default: break;  /* there is one value available (somewhere) */
  }
}


/*
** Ensures expression value is in register 'reg' (and therefore
** 'e' will become a non-relocatable expression).
*/
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
      NameDef(Instruction) *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      break;
    }
    case NameDef(VNONRELOC): {
      if (reg != e->u.info)
        NameDef(luaK_codeABC)(fs, NameDef(OP_MOVE), reg, e->u.info, 0);
      break;
    }
    default: {
      lua_assert(e->k == NameDef(VJMP));
      return;  /* nothing to do... */
    }
  }
  e->u.info = reg;
  e->k = NameDef(VNONRELOC);
}


/*
** Ensures expression value is in any register.
*/
static void discharge2anyreg (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k != NameDef(VNONRELOC)) {  /* no fixed register yet? */
    NameDef(luaK_reserveregs)(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


static int code_loadbool (NameDef(FuncState) *fs, int A, int b, int jump) {
  NameDef(luaK_getlabel)(fs);  /* those instructions may be jump targets */
  return NameDef(luaK_codeABC)(fs, NameDef(OP_LOADBOOL), A, b, jump);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
static int need_value (NameDef(FuncState) *fs, int list) {
  for (; list != NO_JUMP; list = getjump(fs, list)) {
    NameDef(Instruction) i = *getjumpcontrol(fs, list);
    if (GET_OPCODE(i) != NameDef(OP_TESTSET)) return 1;
  }
  return 0;  /* not found */
}


/*
** Ensures final expression result (including results from its jump
** lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
static void exp2reg (NameDef(FuncState) *fs, NameDef(expdesc) *e, int reg) {
  discharge2reg(fs, e, reg);
  if (e->k == NameDef(VJMP))  /* expression itself is a test? */
    NameDef(luaK_concat)(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  if (hasjumps(e)) {
    int final;  /* position after whole expression */
    int p_f = NO_JUMP;  /* position of an eventual LOAD false */
    int p_t = NO_JUMP;  /* position of an eventual LOAD true */
    if (need_value(fs, e->t) || need_value(fs, e->f)) {
      int fj = (e->k == NameDef(VJMP)) ? NO_JUMP : NameDef(luaK_jump)(fs);
      p_f = code_loadbool(fs, reg, 0, 1);
      p_t = code_loadbool(fs, reg, 1, 0);
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


/*
** Ensures final expression result (including results from its jump
** lists) is in next available register.
*/
void NameDef(luaK_exp2nextreg) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_dischargevars)(fs, e);
  freeexp(fs, e);
  NameDef(luaK_reserveregs)(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result (including results from its jump
** lists) is in some (any) register and return that register.
*/
int NameDef(luaK_exp2anyreg) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_dischargevars)(fs, e);
  if (e->k == NameDef(VNONRELOC)) {  /* expression already has a register? */
    if (!hasjumps(e))  /* no jumps? */
      return e->u.info;  /* result is already in a register */
    if (e->u.info >= fs->nactvar) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      return e->u.info;
    }
  }
  NameDef(luaK_exp2nextreg)(fs, e);  /* otherwise, use next available register */
  return e->u.info;
}


/*
** Ensures final expression result is either in a register or in an
** upvalue.
*/
void NameDef(luaK_exp2anyregup) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (e->k != NameDef(VUPVAL) || hasjumps(e))
    NameDef(luaK_exp2anyreg)(fs, e);
}


/*
** Ensures final expression result is either in a register or it is
** a constant.
*/
void NameDef(luaK_exp2val) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  if (hasjumps(e))
    NameDef(luaK_exp2anyreg)(fs, e);
  else
    NameDef(luaK_dischargevars)(fs, e);
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns R/K index.
*/  
int NameDef(luaK_exp2RK) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_exp2val)(fs, e);
  switch (e->k) {  /* move constants to 'k' */
    case NameDef(VTRUE): e->u.info = boolK(fs, 1); goto vk;
    case NameDef(VFALSE): e->u.info = boolK(fs, 0); goto vk;
    case NameDef(VNIL): e->u.info = nilK(fs); goto vk;
    case NameDef(VKINT): e->u.info = NameDef(luaK_intK)(fs, e->u.ival); goto vk;
    case NameDef(VKFLT): e->u.info = luaK_numberK(fs, e->u.nval); goto vk;
    case NameDef(VK):
     vk:
      e->k = NameDef(VK);
      if (e->u.info <= MAXINDEXRK)  /* constant fits in 'argC'? */
        return RKASK(e->u.info);
      else break;
    default: break;
  }
  /* not a constant in the right range: put it in a register */
  return NameDef(luaK_exp2anyreg)(fs, e);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
void NameDef(luaK_storevar) (NameDef(FuncState) *fs, NameDef(expdesc) *var, NameDef(expdesc) *ex) {
  switch (var->k) {
    case NameDef(VLOCAL): {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.info);  /* compute 'ex' into proper place */
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
    default: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Emit SELF instruction (convert expression 'e' into 'e:key(e,').
*/
void NameDef(luaK_self) (NameDef(FuncState) *fs, NameDef(expdesc) *e, NameDef(expdesc) *key) {
  int ereg;
  NameDef(luaK_exp2anyreg)(fs, e);
  ereg = e->u.info;  /* register where 'e' was placed */
  freeexp(fs, e);
  e->u.info = fs->freereg;  /* base register for op_self */
  e->k = NameDef(VNONRELOC);  /* self expression has a fixed register */
  NameDef(luaK_reserveregs)(fs, 2);  /* function and 'self' produced by op_self */
  NameDef(luaK_codeABC)(fs, NameDef(OP_SELF), e->u.info, ereg, NameDef(luaK_exp2RK)(fs, key));
  freeexp(fs, key);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
static void negatecondition (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(Instruction) *pc = getjumpcontrol(fs, e->u.info);
  lua_assert(testTMode(GET_OPCODE(*pc)) && GET_OPCODE(*pc) != OP_TESTSET &&
                                           GET_OPCODE(*pc) != OP_TEST);
  SETARG_A(*pc, !(GETARG_A(*pc)));
}


/*
** Emit instruction to jump if 'e' is 'cond' (that is, if 'cond'
** is true, code will jump if 'e' is true.) Return jump position.
** Optimize when 'e' is 'not' something, inverting the condition
** and removing the 'not'.
*/
static int jumponcond (NameDef(FuncState) *fs, NameDef(expdesc) *e, int cond) {
  if (e->k == NameDef(VRELOCABLE)) {
    NameDef(Instruction) ie = getinstruction(fs, e);
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


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
void NameDef(luaK_goiftrue) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  int pc;  /* pc of new jump */
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VJMP): {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      break;
    }
    case NameDef(VK): case NameDef(VKFLT): case NameDef(VKINT): case NameDef(VTRUE): {
      pc = NO_JUMP;  /* always true; do nothing */
      break;
    }
    default: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      break;
    }
  }
  NameDef(luaK_concat)(fs, &e->f, pc);  /* insert new jump in false list */
  NameDef(luaK_patchtohere)(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
void NameDef(luaK_goiffalse) (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  int pc;  /* pc of new jump */
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VJMP): {
      pc = e->u.info;  /* already jump if true */
      break;
    }
    case NameDef(VNIL): case NameDef(VFALSE): {
      pc = NO_JUMP;  /* always false; do nothing */
      break;
    }
    default: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      break;
    }
  }
  NameDef(luaK_concat)(fs, &e->t, pc);  /* insert new jump in 't' list */
  NameDef(luaK_patchtohere)(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
static void codenot (NameDef(FuncState) *fs, NameDef(expdesc) *e) {
  NameDef(luaK_dischargevars)(fs, e);
  switch (e->k) {
    case NameDef(VNIL): case NameDef(VFALSE): {
      e->k = NameDef(VTRUE);  /* true == not nil == not false */
      break;
    }
    case NameDef(VK): case NameDef(VKFLT): case NameDef(VKINT): case NameDef(VTRUE): {
      e->k = NameDef(VFALSE);  /* false == not "x" == not 0.5 == not 1 == not true */
      break;
    }
    case NameDef(VJMP): {
      negatecondition(fs, e);
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
    default: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { int temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue.
*/
void NameDef(luaK_indexed) (NameDef(FuncState) *fs, NameDef(expdesc) *t, NameDef(expdesc) *k) {
  lua_assert(!hasjumps(t) && (vkisinreg(t->k) || t->k == VUPVAL));
  t->u.ind.t = t->u.info;  /* register or upvalue index */
  t->u.ind.idx = NameDef(luaK_exp2RK)(fs, k);  /* R/K index for key */
  t->u.ind.vt = (t->k == NameDef(VUPVAL)) ? NameDef(VUPVAL) : NameDef(VLOCAL);
  t->k = NameDef(VINDEXED);
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
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
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
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
  else {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    NameDef(lua_Number) n = fltvalue(&res);
    if (luai_numisnan(n) || n == 0)
      return 0;
    e1->k = NameDef(VKFLT);
    e1->u.nval = n;
  }
  return 1;
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
static void codeunexpval (NameDef(FuncState) *fs, NameDef(OpCode) op, NameDef(expdesc) *e, int line) {
  int r = NameDef(luaK_exp2anyreg)(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = NameDef(luaK_codeABC)(fs, op, 0, r, 0);  /* generate opcode */
  e->k = NameDef(VRELOCABLE);  /* all those operations are relocatable */
  NameDef(luaK_fixline)(fs, line);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
static void codebinexpval (NameDef(FuncState) *fs, NameDef(OpCode) op,
                           NameDef(expdesc) *e1, NameDef(expdesc) *e2, int line) {
  int rk1 = NameDef(luaK_exp2RK)(fs, e1);  /* both operands are "RK" */
  int rk2 = NameDef(luaK_exp2RK)(fs, e2);
  freeexps(fs, e1, e2);
  e1->u.info = NameDef(luaK_codeABC)(fs, op, 0, rk1, rk2);  /* generate opcode */
  e1->k = NameDef(VRELOCABLE);  /* all those operations are relocatable */
  NameDef(luaK_fixline)(fs, line);
}


/*
** Emit code for comparisons.
** 'e1' was already put in R/K form by 'luaK_infix'.
*/
static void codecomp (NameDef(FuncState) *fs, NameDef(BinOpr) opr, NameDef(expdesc) *e1, NameDef(expdesc) *e2) {
  int rk1 = (e1->k == NameDef(VK)) ? RKASK(e1->u.info)
                          : check_exp(e1->k == NameDef(VNONRELOC), e1->u.info);
  int rk2 = NameDef(luaK_exp2RK)(fs, e2);
  freeexps(fs, e1, e2);
  switch (opr) {
    case NameDef(OPR_NE): {  /* '(a ~= b)' ==> 'not (a == b)' */
      e1->u.info = condjump(fs, NameDef(OP_EQ), 0, rk1, rk2);
      break;
    }
    case NameDef(OPR_GT): case NameDef(OPR_GE): {
      /* '(a > b)' ==> '(b < a)';  '(a >= b)' ==> '(b <= a)' */
      NameDef(OpCode) op = cast(NameDef(OpCode), (opr - NameDef(OPR_NE)) + NameDef(OP_EQ));
      e1->u.info = condjump(fs, op, 1, rk2, rk1);  /* invert operands */
      break;
    }
    default: {  /* '==', '<', '<=' use their own opcodes */
      NameDef(OpCode) op = cast(NameDef(OpCode), (opr - NameDef(OPR_EQ)) + NameDef(OP_EQ));
      e1->u.info = condjump(fs, op, 1, rk1, rk2);
      break;
    }
  }
  e1->k = NameDef(VJMP);
}


/*
** Aplly prefix operation 'op' to expression 'e'.
*/
void NameDef(luaK_prefix) (NameDef(FuncState) *fs, NameDef(UnOpr) op, NameDef(expdesc) *e, int line) {
  static NameDef(expdesc) ef = {NameDef(VKINT), {0}, NO_JUMP, NO_JUMP};  /* fake 2nd operand */
  switch (op) {
    case NameDef(OPR_MINUS): case NameDef(OPR_BNOT):
      if (constfolding(fs, op + LUA_OPUNM, e, &ef))
        break;
      /* FALLTHROUGH */
    case NameDef(OPR_LEN):
      codeunexpval(fs, cast(NameDef(OpCode), op + NameDef(OP_UNM)), e, line);
      break;
    case NameDef(OPR_NOT): codenot(fs, e); break;
    default: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
void NameDef(luaK_infix) (NameDef(FuncState) *fs, NameDef(BinOpr) op, NameDef(expdesc) *v) {
  switch (op) {
    case NameDef(OPR_AND): {
      NameDef(luaK_goiftrue)(fs, v);  /* go ahead only if 'v' is true */
      break;
    }
    case NameDef(OPR_OR): {
      NameDef(luaK_goiffalse)(fs, v);  /* go ahead only if 'v' is false */
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
      if (!tonumeral(v, NULL))
        NameDef(luaK_exp2RK)(fs, v);
      /* else keep numeral, which may be folded with 2nd operand */
      break;
    }
    default: {
      NameDef(luaK_exp2RK)(fs, v);
      break;
    }
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
** For '(a .. b .. c)' (which is '(a .. (b .. c))', because
** concatenation is right associative), merge second CONCAT into first
** one.
*/
void NameDef(luaK_posfix) (NameDef(FuncState) *fs, NameDef(BinOpr) op,
                  NameDef(expdesc) *e1, NameDef(expdesc) *e2, int line) {
  switch (op) {
    case NameDef(OPR_AND): {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luK_infix' */
      NameDef(luaK_dischargevars)(fs, e2);
      NameDef(luaK_concat)(fs, &e2->f, e1->f);
      *e1 = *e2;
      break;
    }
    case NameDef(OPR_OR): {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luK_infix' */
      NameDef(luaK_dischargevars)(fs, e2);
      NameDef(luaK_concat)(fs, &e2->t, e1->t);
      *e1 = *e2;
      break;
    }
    case NameDef(OPR_CONCAT): {
      NameDef(luaK_exp2val)(fs, e2);
      if (e2->k == NameDef(VRELOCABLE) &&
          GET_OPCODE(getinstruction(fs, e2)) == NameDef(OP_CONCAT)) {
        lua_assert(e1->u.info == GETARG_B(getinstruction(fs, e2))-1);
        freeexp(fs, e1);
        SETARG_B(getinstruction(fs, e2), e1->u.info);
        e1->k = NameDef(VRELOCABLE); e1->u.info = e2->u.info;
      }
      else {
        NameDef(luaK_exp2nextreg)(fs, e2);  /* operand must be on the 'stack' */
        codebinexpval(fs, NameDef(OP_CONCAT), e1, e2, line);
      }
      break;
    }
    case NameDef(OPR_ADD): case NameDef(OPR_SUB): case NameDef(OPR_MUL): case NameDef(OPR_DIV):
    case NameDef(OPR_IDIV): case NameDef(OPR_MOD): case NameDef(OPR_POW):
    case NameDef(OPR_BAND): case NameDef(OPR_BOR): case NameDef(OPR_BXOR):
    case NameDef(OPR_SHL): case NameDef(OPR_SHR): {
      if (!constfolding(fs, op + LUA_OPADD, e1, e2))
        codebinexpval(fs, cast(NameDef(OpCode), op + NameDef(OP_ADD)), e1, e2, line);
      break;
    }
    case NameDef(OPR_EQ): case NameDef(OPR_LT): case NameDef(OPR_LE):
    case NameDef(OPR_NE): case NameDef(OPR_GT): case NameDef(OPR_GE): {
      codecomp(fs, op, e1, e2);
      break;
    }
    default: lua_assert(0);
  }
}


/*
** Change line information associated with current position.
*/
void NameDef(luaK_fixline) (NameDef(FuncState) *fs, int line) {
  fs->f->lineinfo[fs->pc - 1] = line;
}


/*
** Emit a SETLIST instruction.
** 'base' is register that keeps table;
** 'nelems' is #table plus those to be stored now;
** 'tostore' is number of values (in registers 'base + 1',...) to add to
** table (or LUA_MULTRET to add up to stack top).
*/
void NameDef(luaK_setlist) (NameDef(FuncState) *fs, int base, int nelems, int tostore) {
  int c =  (nelems - 1)/LFIELDS_PER_FLUSH + 1;
  int b = (tostore == LUA_MULTRET) ? 0 : tostore;
  lua_assert(tostore != 0 && tostore <= LFIELDS_PER_FLUSH);
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

