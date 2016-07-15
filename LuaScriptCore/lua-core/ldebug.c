/*
** $Id: ldebug.c,v 2.115 2015/05/22 17:45:56 roberto Exp $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#include "LuaDefine.h"

#include "lprefix.h"


#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"



#define noLuaClosure(f)		((f) == NULL || (f)->c.tt == LUA_TCCL)


/* Active Lua function (given call info) */
#define ci_func(ci)		(clLvalue((ci)->func))


static const char *getfuncname (NameDef(lua_State) *L, NameDef(CallInfo) *ci, const char **name);


static int currentpc (NameDef(CallInfo) *ci) {
  lua_assert(isLua(ci));
  return pcRel(ci->u.l.savedpc, ci_func(ci)->p);
}


static int currentline (NameDef(CallInfo) *ci) {
  return getfuncline(ci_func(ci)->p, currentpc(ci));
}


/*
** If function yielded, its 'func' can be in the 'extra' field. The
** next function restores 'func' to its correct value for debugging
** purposes. (It exchanges 'func' and 'extra'; so, when called again,
** after debugging, it also "re-restores" ** 'func' to its altered value.
*/
static void swapextra (NameDef(lua_State) *L) {
  if (L->status == LUA_YIELD) {
    NameDef(CallInfo) *ci = L->ci;  /* get function that yielded */
    NameDef(StkId) temp = ci->func;  /* exchange its 'func' and 'extra' values */
    ci->func = restorestack(L, ci->extra);
    ci->extra = savestack(L, temp);
  }
}


/*
** this function can be called asynchronous (e.g. during a signal)
*/
LUA_API void NameDef(lua_sethook) (NameDef(lua_State) *L, NameDef(lua_Hook) func, int mask, int count) {
  if (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  if (isLua(L->ci))
    L->oldpc = L->ci->u.l.savedpc;
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
}


LUA_API NameDef(lua_Hook) NameDef(lua_gethook) (NameDef(lua_State) *L) {
  return L->hook;
}


LUA_API int NameDef(lua_gethookmask) (NameDef(lua_State) *L) {
  return L->hookmask;
}


LUA_API int NameDef(lua_gethookcount) (NameDef(lua_State) *L) {
  return L->basehookcount;
}


LUA_API int NameDef(lua_getstack) (NameDef(lua_State) *L, int level, NameDef(lua_Debug) *ar) {
  int status;
  NameDef(CallInfo) *ci;
  if (level < 0) return 0;  /* invalid (negative) level */
  lua_lock(L);
  for (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  if (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  else status = 0;  /* no such level */
  lua_unlock(L);
  return status;
}


static const char *upvalname (NameDef(Proto) *p, int uv) {
  NameDef(TString) *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  if (s == NULL) return "?";
  else return getstr(s);
}


static const char *findvararg (NameDef(CallInfo) *ci, int n, NameDef(StkId) *pos) {
  int nparams = clLvalue(ci->func)->p->numparams;
  if (n >= cast_int(ci->u.l.base - ci->func) - nparams)
    return NULL;  /* no such vararg */
  else {
    *pos = ci->func + nparams + n;
    return "(*vararg)";  /* generic name for any vararg */
  }
}


static const char *findlocal (NameDef(lua_State) *L, NameDef(CallInfo) *ci, int n,
                              NameDef(StkId) *pos) {
  const char *name = NULL;
  NameDef(StkId) base;
  if (isLua(ci)) {
    if (n < 0)  /* access to vararg values? */
      return findvararg(ci, -n, pos);
    else {
      base = ci->u.l.base;
      name = NameDef(luaF_getlocalname)(ci_func(ci)->p, n, currentpc(ci));
    }
  }
  else
    base = ci->func + 1;
  if (name == NULL) {  /* no 'standard' name? */
    NameDef(StkId) limit = (ci == L->ci) ? L->top : ci->next->func;
    if (limit - base >= n && n > 0)  /* is 'n' inside 'ci' stack? */
      name = "(*temporary)";  /* generic name for any valid slot */
    else
      return NULL;  /* no name */
  }
  *pos = base + (n - 1);
  return name;
}


LUA_API const char *NameDef(lua_getlocal) (NameDef(lua_State) *L, const NameDef(lua_Debug) *ar, int n) {
  const char *name;
  lua_lock(L);
  swapextra(L);
  if (ar == NULL) {  /* information about non-active function? */
    if (!isLfunction(L->top - 1))  /* not a Lua function? */
      name = NULL;
    else  /* consider live variables at function start (parameters) */
      name = NameDef(luaF_getlocalname)(clLvalue(L->top - 1)->p, n, 0);
  }
  else {  /* active function; get information through 'ar' */
    NameDef(StkId) pos = NULL;  /* to avoid warnings */
    name = findlocal(L, ar->i_ci, n, &pos);
    if (name) {
      setobj2s(L, L->top, pos);
      api_incr_top(L);
    }
  }
  swapextra(L);
  lua_unlock(L);
  return name;
}


LUA_API const char *NameDef(lua_setlocal) (NameDef(lua_State) *L, const NameDef(lua_Debug) *ar, int n) {
  NameDef(StkId) pos = NULL;  /* to avoid warnings */
  const char *name;
  lua_lock(L);
  swapextra(L);
  name = findlocal(L, ar->i_ci, n, &pos);
  if (name) {
    setobjs2s(L, pos, L->top - 1);
    L->top--;  /* pop value */
  }
  swapextra(L);
  lua_unlock(L);
  return name;
}


static void funcinfo (NameDef(lua_Debug) *ar, NameDef(Closure) *cl) {
  if (noLuaClosure(cl)) {
    ar->source = "=[C]";
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  else {
    NameDef(Proto) *p = cl->l.p;
    ar->source = p->source ? getstr(p->source) : "=?";
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  NameDef(luaO_chunkid)(ar->short_src, ar->source, LUA_IDSIZE);
}


static void collectvalidlines (NameDef(lua_State) *L, NameDef(Closure) *f) {
  if (noLuaClosure(f)) {
    setnilvalue(L->top);
    api_incr_top(L);
  }
  else {
    int i;
    NameDef(TValue) v;
    int *lineinfo = f->l.p->lineinfo;
    NameDef(Table) *t = NameDef(luaH_new)(L);  /* new table to store active lines */
    sethvalue(L, L->top, t);  /* push it on stack */
    api_incr_top(L);
    setbvalue(&v, 1);  /* boolean 'true' to be the value of all indices */
    for (i = 0; i < f->l.p->sizelineinfo; i++)  /* for all lines with code */
      NameDef(luaH_setint)(L, t, lineinfo[i], &v);  /* table[line] = true */
  }
}


static int auxgetinfo (NameDef(lua_State) *L, const char *what, NameDef(lua_Debug) *ar,
                       NameDef(Closure) *f, NameDef(CallInfo) *ci) {
  int status = 1;
  for (; *what; what++) {
    switch (*what) {
      case 'S': {
        funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->currentline = (ci && isLua(ci)) ? currentline(ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = (f == NULL) ? 0 : f->c.nupvalues;
        if (noLuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        else {
          ar->isvararg = f->l.p->is_vararg;
          ar->nparams = f->l.p->numparams;
        }
        break;
      }
      case 't': {
        ar->istailcall = (ci) ? ci->callstatus & CIST_TAIL : 0;
        break;
      }
      case 'n': {
        /* calling function is a known Lua function? */
        if (ci && !(ci->callstatus & CIST_TAIL) && isLua(ci->previous))
          ar->namewhat = getfuncname(L, ci->previous, &ar->name);
        else
          ar->namewhat = NULL;
        if (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        break;
      }
      case 'L':
      case 'f':  /* handled by lua_getinfo */
        break;
      default: status = 0;  /* invalid option */
    }
  }
  return status;
}


LUA_API int NameDef(lua_getinfo) (NameDef(lua_State) *L, const char *what, NameDef(lua_Debug) *ar) {
  int status;
  NameDef(Closure) *cl;
  NameDef(CallInfo) *ci;
  NameDef(StkId) func;
  lua_lock(L);
  swapextra(L);
  if (*what == '>') {
    ci = NULL;
    func = L->top - 1;
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top--;  /* pop function */
  }
  else {
    ci = ar->i_ci;
    func = ci->func;
    lua_assert(ttisfunction(ci->func));
  }
  cl = ttisclosure(func) ? clvalue(func) : NULL;
  status = auxgetinfo(L, what, ar, cl, ci);
  if (strchr(what, 'f')) {
    setobjs2s(L, L->top, func);
    api_incr_top(L);
  }
  swapextra(L);  /* correct before option 'L', which can raise a mem. error */
  if (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  return status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/

static const char *getobjname (NameDef(Proto) *p, int lastpc, int reg,
                               const char **name);


/*
** find a "name" for the RK value 'c'
*/
static void kname (NameDef(Proto) *p, int pc, int c, const char **name) {
  if (ISK(c)) {  /* is 'c' a constant? */
    NameDef(TValue) *kvalue = &p->k[INDEXK(c)];
    if (ttisstring(kvalue)) {  /* literal constant? */
      *name = svalue(kvalue);  /* it is its own name */
      return;
    }
    /* else no reasonable name found */
  }
  else {  /* 'c' is a register */
    const char *what = getobjname(p, pc, c, name); /* search for 'c' */
    if (what && *what == 'c') {  /* found a constant name? */
      return;  /* 'name' already filled */
    }
    /* else no reasonable name found */
  }
  *name = "?";  /* no reasonable name found */
}


static int filterpc (int pc, int jmptarget) {
  if (pc < jmptarget)  /* is code conditional (inside a jump)? */
    return -1;  /* cannot know who sets that register */
  else return pc;  /* current position sets that register */
}


/*
** try to find last instruction before 'lastpc' that modified register 'reg'
*/
static int findsetreg (NameDef(Proto) *p, int lastpc, int reg) {
  int pc;
  int setreg = -1;  /* keep last instruction that changed 'reg' */
  int jmptarget = 0;  /* any code before this address is conditional */
  for (pc = 0; pc < lastpc; pc++) {
    NameDef(Instruction) i = p->code[pc];
    NameDef(OpCode) op = GET_OPCODE(i);
    int a = GETARG_A(i);
    switch (op) {
      case NameDef(OP_LOADNIL): {
        int b = GETARG_B(i);
        if (a <= reg && reg <= a + b)  /* set registers from 'a' to 'a+b' */
          setreg = filterpc(pc, jmptarget);
        break;
      }
      case NameDef(OP_TFORCALL): {
        if (reg >= a + 2)  /* affect all regs above its base */
          setreg = filterpc(pc, jmptarget);
        break;
      }
      case NameDef(OP_CALL):
      case NameDef(OP_TAILCALL): {
        if (reg >= a)  /* affect all registers above base */
          setreg = filterpc(pc, jmptarget);
        break;
      }
      case NameDef(OP_JMP): {
        int b = GETARG_sBx(i);
        int dest = pc + 1 + b;
        /* jump is forward and do not skip 'lastpc'? */
        if (pc < dest && dest <= lastpc) {
          if (dest > jmptarget)
            jmptarget = dest;  /* update 'jmptarget' */
        }
        break;
      }
      default:
        if (testAMode(op) && reg == a)  /* any instruction that set A */
          setreg = filterpc(pc, jmptarget);
        break;
    }
  }
  return setreg;
}


static const char *getobjname (NameDef(Proto) *p, int lastpc, int reg,
                               const char **name) {
  int pc;
  *name = NameDef(luaF_getlocalname)(p, reg + 1, lastpc);
  if (*name)  /* is a local? */
    return "local";
  /* else try symbolic execution */
  pc = findsetreg(p, lastpc, reg);
  if (pc != -1) {  /* could find instruction? */
    NameDef(Instruction) i = p->code[pc];
    NameDef(OpCode) op = GET_OPCODE(i);
    switch (op) {
      case NameDef(OP_MOVE): {
        int b = GETARG_B(i);  /* move from 'b' to 'a' */
        if (b < GETARG_A(i))
          return getobjname(p, pc, b, name);  /* get name for 'b' */
        break;
      }
      case NameDef(OP_GETTABUP):
      case NameDef(OP_GETTABLE): {
        int k = GETARG_C(i);  /* key index */
        int t = GETARG_B(i);  /* table index */
        const char *vn = (op == NameDef(OP_GETTABLE))  /* name of indexed variable */
                         ? NameDef(luaF_getlocalname)(p, t + 1, pc)
                         : upvalname(p, t);
        kname(p, pc, k, name);
        return (vn && strcmp(vn, LUA_ENV) == 0) ? "global" : "field";
      }
      case NameDef(OP_GETUPVAL): {
        *name = upvalname(p, GETARG_B(i));
        return "upvalue";
      }
      case NameDef(OP_LOADK):
      case NameDef(OP_LOADKX): {
        int b = (op == NameDef(OP_LOADK)) ? GETARG_Bx(i)
                                 : GETARG_Ax(p->code[pc + 1]);
        if (ttisstring(&p->k[b])) {
          *name = svalue(&p->k[b]);
          return "constant";
        }
        break;
      }
      case NameDef(OP_SELF): {
        int k = GETARG_C(i);  /* key index */
        kname(p, pc, k, name);
        return "method";
      }
      default: break;  /* go through to return NULL */
    }
  }
  return NULL;  /* could not find reasonable name */
}


static const char *getfuncname (NameDef(lua_State) *L, NameDef(CallInfo) *ci, const char **name) {
  NameDef(TMS) tm = (NameDef(TMS))0;  /* to avoid warnings */
  NameDef(Proto) *p = ci_func(ci)->p;  /* calling function */
  int pc = currentpc(ci);  /* calling instruction index */
  NameDef(Instruction) i = p->code[pc];  /* calling instruction */
  if (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    return "hook";
  }
  switch (GET_OPCODE(i)) {
    case NameDef(OP_CALL):
    case NameDef(OP_TAILCALL):  /* get function name */
      return getobjname(p, pc, GETARG_A(i), name);
    case NameDef(OP_TFORCALL): {  /* for iterator */
      *name = "for iterator";
       return "for iterator";
    }
    /* all other instructions can call only through metamethods */
    case NameDef(OP_SELF): case NameDef(OP_GETTABUP): case NameDef(OP_GETTABLE):
      tm = NameDef(TM_INDEX);
      break;
    case NameDef(OP_SETTABUP): case NameDef(OP_SETTABLE):
      tm = NameDef(TM_NEWINDEX);
      break;
    case NameDef(OP_ADD): case NameDef(OP_SUB): case NameDef(OP_MUL): case NameDef(OP_MOD):
    case NameDef(OP_POW): case NameDef(OP_DIV): case NameDef(OP_IDIV): case NameDef(OP_BAND):
    case NameDef(OP_BOR): case NameDef(OP_BXOR): case NameDef(OP_SHL): case NameDef(OP_SHR): {
      int offset = cast_int(GET_OPCODE(i)) - cast_int(NameDef(OP_ADD));  /* ORDER OP */
      tm = cast(NameDef(TMS), offset + cast_int(NameDef(TM_ADD)));  /* ORDER TM */
      break;
    }
    case NameDef(OP_UNM): tm = NameDef(TM_UNM); break;
    case NameDef(OP_BNOT): tm = NameDef(TM_BNOT); break;
    case NameDef(OP_LEN): tm = NameDef(TM_LEN); break;
    case NameDef(OP_CONCAT): tm = NameDef(TM_CONCAT); break;
    case NameDef(OP_EQ): tm = NameDef(TM_EQ); break;
    case NameDef(OP_LT): tm = NameDef(TM_LT); break;
    case NameDef(OP_LE): tm = NameDef(TM_LE); break;
    default: lua_assert(0);  /* other instructions cannot call a function */
  }
  *name = getstr(G(L)->tmname[tm]);
  return "metamethod";
}

/* }====================================================== */



/*
** The subtraction of two potentially unrelated pointers is
** not ISO C, but it should not crash a program; the subsequent
** checks are ISO C and ensure a correct result.
*/
static int isinstack (NameDef(CallInfo) *ci, const NameDef(TValue) *o) {
  ptrdiff_t i = o - ci->u.l.base;
  return (0 <= i && i < (ci->top - ci->u.l.base) && ci->u.l.base + i == o);
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const char *getupvalname (NameDef(CallInfo) *ci, const NameDef(TValue) *o,
                                 const char **name) {
  NameDef(LClosure) *c = ci_func(ci);
  int i;
  for (i = 0; i < c->nupvalues; i++) {
    if (c->upvals[i]->v == o) {
      *name = upvalname(c->p, i);
      return "upvalue";
    }
  }
  return NULL;
}


static const char *varinfo (NameDef(lua_State) *L, const NameDef(TValue) *o) {
  const char *name = NULL;  /* to avoid warnings */
  NameDef(CallInfo) *ci = L->ci;
  const char *kind = NULL;
  if (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    if (!kind && isinstack(ci, o))  /* no? try a register */
      kind = getobjname(ci_func(ci)->p, currentpc(ci),
                        cast_int(o - ci->u.l.base), &name);
  }
  return (kind) ? NameDef(luaO_pushfstring)(L, " (%s '%s')", kind, name) : "";
}


l_noret NameDef(luaG_typeerror) (NameDef(lua_State) *L, const NameDef(TValue) *o, const char *op) {
  const char *t = objtypename(o);
  NameDef(luaG_runerror)(L, "attempt to %s a %s value%s", op, t, varinfo(L, o));
}


l_noret NameDef(luaG_concaterror) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2) {
  if (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  NameDef(luaG_typeerror)(L, p1, "concatenate");
}


l_noret NameDef(luaG_opinterror) (NameDef(lua_State) *L, const NameDef(TValue) *p1,
                         const NameDef(TValue) *p2, const char *msg) {
  NameDef(lua_Number) temp;
  if (!tonumber(p1, &temp))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  NameDef(luaG_typeerror)(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret NameDef(luaG_tointerror) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2) {
  NameDef(lua_Integer) temp;
  if (!tointeger(p1, &temp))
    p2 = p1;
  NameDef(luaG_runerror)(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret NameDef(luaG_ordererror) (NameDef(lua_State) *L, const NameDef(TValue) *p1, const NameDef(TValue) *p2) {
  const char *t1 = objtypename(p1);
  const char *t2 = objtypename(p2);
  if (t1 == t2)
    NameDef(luaG_runerror)(L, "attempt to compare two %s values", t1);
  else
    NameDef(luaG_runerror)(L, "attempt to compare %s with %s", t1, t2);
}


/* add src:line information to 'msg' */
const char *NameDef(luaG_addinfo) (NameDef(lua_State) *L, const char *msg, NameDef(TString) *src,
                                        int line) {
  char buff[LUA_IDSIZE];
  if (src)
    NameDef(luaO_chunkid)(buff, getstr(src), LUA_IDSIZE);
  else {  /* no source available; use "?" instead */
    buff[0] = '?'; buff[1] = '\0';
  }
  return NameDef(luaO_pushfstring)(L, "%s:%d: %s", buff, line, msg);
}


l_noret NameDef(luaG_errormsg) (NameDef(lua_State) *L) {
  if (L->errfunc != 0) {  /* is there an error handling function? */
    NameDef(StkId) errfunc = restorestack(L, L->errfunc);
    setobjs2s(L, L->top, L->top - 1);  /* move argument */
    setobjs2s(L, L->top - 1, errfunc);  /* push function */
    L->top++;  /* assume EXTRA_STACK */
    NameDef(luaD_call)(L, L->top - 2, 1, 0);  /* call it */
  }
  NameDef(luaD_throw)(L, LUA_ERRRUN);
}


l_noret NameDef(luaG_runerror) (NameDef(lua_State) *L, const char *fmt, ...) {
  NameDef(CallInfo) *ci = L->ci;
  const char *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = NameDef(luaO_pushvfstring)(L, fmt, argp);  /* format message */
  va_end(argp);
  if (isLua(ci))  /* if Lua function, add source:line information */
    NameDef(luaG_addinfo)(L, msg, ci_func(ci)->p->source, currentline(ci));
  NameDef(luaG_errormsg)(L);
}


void NameDef(luaG_traceexec) (NameDef(lua_State) *L) {
  NameDef(CallInfo) *ci = L->ci;
  NameDef(lu_byte) mask = L->hookmask;
  int counthook = ((mask & LUA_MASKCOUNT) && L->hookcount == 0);
  if (counthook)
    resethookcount(L);  /* reset count */
  if (ci->callstatus & CIST_HOOKYIELD) {  /* called hook last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    return;  /* do not call hook again (VM yielded, so it did not move) */
  }
  if (counthook)
    NameDef(luaD_hook)(L, LUA_HOOKCOUNT, -1);  /* call count hook */
  if (mask & LUA_MASKLINE) {
    NameDef(Proto) *p = ci_func(ci)->p;
    int npc = pcRel(ci->u.l.savedpc, p);
    int newline = getfuncline(p, npc);
    if (npc == 0 ||  /* call linehook when enter a new function, */
        ci->u.l.savedpc <= L->oldpc ||  /* when jump back (loop), or when */
        newline != getfuncline(p, pcRel(L->oldpc, p)))  /* enter a new line */
      NameDef(luaD_hook)(L, LUA_HOOKLINE, newline);  /* call line hook */
  }
  L->oldpc = ci->u.l.savedpc;
  if (L->status == LUA_YIELD) {  /* did hook yield? */
    if (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->u.l.savedpc--;  /* undo increment (resume will increment it again) */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    ci->func = L->top - 1;  /* protect stack below results */
    NameDef(luaD_throw)(L, LUA_YIELD);
  }
}

