/*
** $Id: lstate.h,v 2.130 2015/12/16 16:39:38 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#ifndef lstate_h
#define lstate_h

#include "lua.h"

#include "lobject.h"
#include "ltm.h"
#include "lzio.h"


/*

** Some notes about garbage-collected objects: All objects in Lua must
** be kept somehow accessible until being freed, so all objects always
** belong to one (and only one) of these lists, using field 'next' of
** the 'CommonHeader' for the link:
**
** 'allgc': all objects not marked for finalization;
** 'finobj': all objects marked for finalization;
** 'tobefnz': all objects ready to be finalized; 
** 'fixedgc': all objects that are not to be collected (currently
** only small strings, such as reserved words).

*/


struct NameDef(lua_longjmp);  /* defined in ldo.c */


/*
** Atomic type (relative to signals) to better ensure that 'lua_sethook' 
** is thread safe
*/
#if !defined(l_signalT)
#include <signal.h>
#define l_signalT	sig_atomic_t
#endif


/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)


/* kinds of Garbage Collection */
#define KGC_NORMAL	0
#define KGC_EMERGENCY	1	/* gc was forced by an allocation failure */


typedef struct NameDef(stringtable) {
  NameDef(TString) **hash;
  int nuse;  /* number of elements */
  int size;
} NameDef(stringtable);


/*
** Information about a call.
** When a thread yields, 'func' is adjusted to pretend that the
** top function has only the yielded values in its stack; in that
** case, the actual 'func' value is saved in field 'extra'. 
** When a function calls another with a continuation, 'extra' keeps
** the function index so that, in case of errors, the continuation
** function can be called with the correct top.
*/
typedef struct NameDef(CallInfo) {
    NameDef(StkId) func;  /* function index in the stack */
  NameDef(StkId)	top;  /* top for this function */
  struct NameDef(CallInfo) *previous, *next;  /* dynamic call link */
  union {
    struct {  /* only for Lua functions */
      NameDef(StkId) base;  /* base for this function */
      const NameDef(Instruction) *savedpc;
    } l;
    struct {  /* only for C functions */
      NameDef(lua_KFunction) k;  /* continuation in case of yields */
      ptrdiff_t old_errfunc;
      NameDef(lua_KContext) ctx;  /* context info. in case of yields */
    } c;
  } u;
  ptrdiff_t extra;
  short nresults;  /* expected number of results from this function */
  NameDef(lu_byte) callstatus;
} NameDef(CallInfo);


/*
** Bits in CallInfo status
*/
#define CIST_OAH	(1<<0)	/* original value of 'allowhook' */
#define CIST_LUA	(1<<1)	/* call is running a Lua function */
#define CIST_HOOKED	(1<<2)	/* call is running a debug hook */
#define CIST_FRESH	(1<<3)	/* call is running on a fresh invocation
                                   of luaV_execute */
#define CIST_YPCALL	(1<<4)	/* call is a yieldable protected call */
#define CIST_TAIL	(1<<5)	/* call was tail called */
#define CIST_HOOKYIELD	(1<<6)	/* last hook called yielded */
#define CIST_LEQ	(1<<7)  /* using __lt for __le */

#define isLua(ci)	((ci)->callstatus & CIST_LUA)

/* assume that CIST_OAH has offset 0 and that 'v' is strictly 0/1 */
#define setoah(st,v)	((st) = ((st) & ~CIST_OAH) | (v))
#define getoah(st)	((st) & CIST_OAH)


/*
** 'global state', shared by all threads of this state
*/
typedef struct NameDef(global_State) {
  NameDef(lua_Alloc) frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to 'frealloc' */
  NameDef(l_mem) totalbytes;  /* number of bytes currently allocated - GCdebt */
  NameDef(l_mem) GCdebt;  /* bytes allocated not yet compensated by the collector */
  NameDef(lu_mem) GCmemtrav;  /* memory traversed by the GC */
  NameDef(lu_mem) GCestimate;  /* an estimate of the non-garbage memory in use */
  NameDef(stringtable) strt;  /* hash table for strings */
  NameDef(TValue) l_registry;
  unsigned int seed;  /* randomized seed for hashes */
  NameDef(lu_byte) currentwhite;
  NameDef(lu_byte) gcstate;  /* state of garbage collector */
  NameDef(lu_byte) gckind;  /* kind of GC running */
  NameDef(lu_byte) gcrunning;  /* true if GC is running */
  NameDef(GCObject) *allgc;  /* list of all collectable objects */
  NameDef(GCObject) **sweepgc;  /* current position of sweep in list */
  NameDef(GCObject) *finobj;  /* list of collectable objects with finalizers */
  NameDef(GCObject) *gray;  /* list of gray objects */
  NameDef(GCObject) *grayagain;  /* list of objects to be traversed atomically */
  NameDef(GCObject) *weak;  /* list of tables with weak values */
  NameDef(GCObject) *ephemeron;  /* list of ephemeron tables (weak keys) */
  NameDef(GCObject) *allweak;  /* list of all-weak tables */
  NameDef(GCObject) *tobefnz;  /* list of userdata to be GC */
  NameDef(GCObject) *fixedgc;  /* list of objects not to be collected */
  struct NameDef(lua_State) *twups;  /* list of threads with open upvalues */
  unsigned int gcfinnum;  /* number of finalizers to call in each GC step */
  int gcpause;  /* size of pause between successive GCs */
  int gcstepmul;  /* GC 'granularity' */
  NameDef(lua_CFunction) panic;  /* to be called in unprotected errors */
  struct NameDef(lua_State) *mainthread;
  const NameDef(lua_Number) *version;  /* pointer to version number */
  NameDef(TString) *memerrmsg;  /* memory-error message */
  NameDef(TString) *tmname[NameDef(TM_N)];  /* array with tag-method names */
  struct NameDef(Table) *mt[LUA_NUMTAGS];  /* metatables for basic types */
  NameDef(TString) *strcache[STRCACHE_N][STRCACHE_M];  /* cache for strings in API */
} NameDef(global_State);


/*
** 'per thread' state
*/
struct NameDef(lua_State) {
  CommonHeader;
  unsigned short nci;  /* number of items in 'ci' list */
  NameDef(lu_byte) status;
  NameDef(StkId) top;  /* first free slot in the stack */
  NameDef(global_State) *l_G;
  NameDef(CallInfo) *ci;  /* call info for current function */
  const NameDef(Instruction) *oldpc;  /* last pc traced */
  NameDef(StkId) stack_last;  /* last free slot in the stack */
  NameDef(StkId) stack;  /* stack base */
  NameDef(UpVal) *openupval;  /* list of open upvalues in this stack */
  NameDef(GCObject) *gclist;
  struct NameDef(lua_State) *twups;  /* list of threads with open upvalues */
  struct NameDef(lua_longjmp) *errorJmp;  /* current error recover point */
  NameDef(CallInfo) base_ci;  /* CallInfo for first level (C calling Lua) */
  volatile NameDef(lua_Hook) hook;
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
  int stacksize;
  int basehookcount;
  int hookcount;
  unsigned short nny;  /* number of non-yieldable calls in stack */
  unsigned short nCcalls;  /* number of nested C calls */
  l_signalT hookmask;
  NameDef(lu_byte) allowhook;
};


#define G(L)	(L->l_G)


/*
** Union of all collectable objects (only for conversions)
*/
union NameDef(GCUnion) {
  NameDef(GCObject) gc;  /* common header */
  struct NameDef(TString) ts;
  struct NameDef(Udata) u;
  union NameDef(Closure) cl;
  struct NameDef(Table) h;
  struct NameDef(Proto) p;
  struct NameDef(lua_State) th;  /* thread */
};


#define cast_u(o)	cast(union NameDef(GCUnion) *, (o))

/* macros to convert a GCObject into a specific value */
#define gco2ts(o)  \
	check_exp(novariant((o)->tt) == LUA_TSTRING, &((cast_u(o))->ts))
#define gco2u(o)  check_exp((o)->tt == LUA_TUSERDATA, &((cast_u(o))->u))
#define gco2lcl(o)  check_exp((o)->tt == LUA_TLCL, &((cast_u(o))->cl.l))
#define gco2ccl(o)  check_exp((o)->tt == LUA_TCCL, &((cast_u(o))->cl.c))
#define gco2cl(o)  \
	check_exp(novariant((o)->tt) == LUA_TFUNCTION, &((cast_u(o))->cl))
#define gco2t(o)  check_exp((o)->tt == LUA_TTABLE, &((cast_u(o))->h))
#define gco2p(o)  check_exp((o)->tt == LUA_TPROTO, &((cast_u(o))->p))
#define gco2th(o)  check_exp((o)->tt == LUA_TTHREAD, &((cast_u(o))->th))


/* macro to convert a Lua object into a GCObject */
#define obj2gco(v) \
	check_exp(novariant((v)->tt) < LUA_TDEADKEY, (&(cast_u(v)->gc)))


/* actual number of total bytes allocated */
#define gettotalbytes(g)	cast(NameDef(lu_mem), (g)->totalbytes + (g)->GCdebt)

LUAI_FUNC void NameDef(luaE_setdebt) (NameDef(global_State) *g, NameDef(l_mem) debt);
LUAI_FUNC void NameDef(luaE_freethread) (NameDef(lua_State) *L, NameDef(lua_State) *L1);
LUAI_FUNC NameDef(CallInfo) *NameDef(luaE_extendCI) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaE_freeCI) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaE_shrinkCI) (NameDef(lua_State) *L);


#endif

