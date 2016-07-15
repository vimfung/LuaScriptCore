/*
** $Id: lobject.h,v 2.111 2015/06/09 14:21:42 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h

#include "LuaDefine.h"

#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	LUA_NUMTAGS
#define LUA_TDEADKEY	(LUA_NUMTAGS+1)

/*
** number of all possible tags (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTAGS	(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* value)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/


/*
** LUA_TFUNCTION variants:
** 0 - Lua function
** 1 - light C function
** 2 - regular C function (closure)
*/

/* Variant tags for functions */
#define LUA_TLCL	(LUA_TFUNCTION | (0 << 4))  /* Lua closure */
#define LUA_TLCF	(LUA_TFUNCTION | (1 << 4))  /* light C function */
#define LUA_TCCL	(LUA_TFUNCTION | (2 << 4))  /* C closure */


/* Variant tags for strings */
#define LUA_TSHRSTR	(LUA_TSTRING | (0 << 4))  /* short strings */
#define LUA_TLNGSTR	(LUA_TSTRING | (1 << 4))  /* long strings */


/* Variant tags for numbers */
#define LUA_TNUMFLT	(LUA_TNUMBER | (0 << 4))  /* float numbers */
#define LUA_TNUMINT	(LUA_TNUMBER | (1 << 4))  /* integer numbers */


/* Bit mark for collectable types */
#define BIT_ISCOLLECTABLE	(1 << 6)

/* mark a tag as collectable */
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)


/*
** Common type for all collectable objects
*/
typedef struct NameDef(GCObject) NameDef(GCObject);


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	NameDef(GCObject) *next; NameDef(lu_byte) tt; NameDef(lu_byte) marked


/*
** Common type has only the common header
*/
struct NameDef(GCObject) {
  CommonHeader;
};



/*
** Union of all Lua values
*/
typedef union NameDef(Value) NameDef(Value);




/*
** Tagged Values. This is the basic representation of values in Lua,
** an actual value plus a tag with its type.
*/

#define TValuefields	NameDef(Value) value_; int tt_

typedef struct NameDef(lua_TValue) NameDef(TValue);


/* macro defining a nil value */
#define NILCONSTANT	{NULL}, LUA_TNIL


#define val_(o)		((o)->value_)


/* raw type tag of a TValue */
#define rttype(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#define novariant(x)	((x) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#define ttype(o)	(rttype(o) & 0x3F)

/* type tag of a TValue with no variants (bits 0-3) */
#define ttnov(o)	(novariant(rttype(o)))


/* Macros to test type */
#define checktag(o,t)		(rttype(o) == (t))
#define checktype(o,t)		(ttnov(o) == (t))
#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
#define ttisfloat(o)		checktag((o), LUA_TNUMFLT)
#define ttisinteger(o)		checktag((o), LUA_TNUMINT)
#define ttisnil(o)		checktag((o), LUA_TNIL)
#define ttisboolean(o)		checktag((o), LUA_TBOOLEAN)
#define ttislightuserdata(o)	checktag((o), LUA_TLIGHTUSERDATA)
#define ttisstring(o)		checktype((o), LUA_TSTRING)
#define ttisshrstring(o)	checktag((o), ctb(LUA_TSHRSTR))
#define ttislngstring(o)	checktag((o), ctb(LUA_TLNGSTR))
#define ttistable(o)		checktag((o), ctb(LUA_TTABLE))
#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#define ttisclosure(o)		((rttype(o) & 0x1F) == LUA_TFUNCTION)
#define ttisCclosure(o)		checktag((o), ctb(LUA_TCCL))
#define ttisLclosure(o)		checktag((o), ctb(LUA_TLCL))
#define ttislcf(o)		checktag((o), LUA_TLCF)
#define ttisfulluserdata(o)	checktag((o), ctb(LUA_TUSERDATA))
#define ttisthread(o)		checktag((o), ctb(LUA_TTHREAD))
#define ttisdeadkey(o)		checktag((o), LUA_TDEADKEY)


/* Macros to access values */
#define ivalue(o)	check_exp(ttisinteger(o), val_(o).i)
#define fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#define nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)))
#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))
#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))
#define clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))
#define fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#define hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))
#define bvalue(o)	check_exp(ttisboolean(o), val_(o).b)
#define thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))
/* a dead value may get the 'gc' field, but cannot access its contents */
#define deadvalue(o)	check_exp(ttisdeadkey(o), cast(void *, val_(o).gc))

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))


#define iscollectable(o)	(rttype(o) & BIT_ISCOLLECTABLE)


/* Macros for internal tests */
#define righttt(obj)		(ttype(obj) == gcvalue(obj)->tt)

#define checkliveness(g,obj) \
	lua_longassert(!iscollectable(obj) || \
			(righttt(obj) && !isdead(g,gcvalue(obj))))


/* Macros to set values */
#define settt_(o,t)	((o)->tt_=(t))

#define setfltvalue(obj,x) \
  { NameDef(TValue) *io=(obj); val_(io).n=(x); settt_(io, LUA_TNUMFLT); }

#define chgfltvalue(obj,x) \
  { NameDef(TValue) *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#define setivalue(obj,x) \
  { NameDef(TValue) *io=(obj); val_(io).i=(x); settt_(io, LUA_TNUMINT); }

#define chgivalue(obj,x) \
  { NameDef(TValue) *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

#define setnilvalue(obj) settt_(obj, LUA_TNIL)

#define setfvalue(obj,x) \
  { NameDef(TValue) *io=(obj); val_(io).f=(x); settt_(io, LUA_TLCF); }

#define setpvalue(obj,x) \
  { NameDef(TValue) *io=(obj); val_(io).p=(x); settt_(io, LUA_TLIGHTUSERDATA); }

#define setbvalue(obj,x) \
  { NameDef(TValue) *io=(obj); val_(io).b=(x); settt_(io, LUA_TBOOLEAN); }

#define setgcovalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(GCObject) *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

#define setsvalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(TString) *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(G(L),io); }

#define setuvalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(Udata) *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TUSERDATA)); \
    checkliveness(G(L),io); }

#define setthvalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(lua_State) *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTHREAD)); \
    checkliveness(G(L),io); }

#define setclLvalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(LClosure) *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TLCL)); \
    checkliveness(G(L),io); }

#define setclCvalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(CClosure) *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TCCL)); \
    checkliveness(G(L),io); }

#define sethvalue(L,obj,x) \
  { NameDef(TValue) *io = (obj); NameDef(Table) *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTABLE)); \
    checkliveness(G(L),io); }

#define setdeadvalue(obj)	settt_(obj, LUA_TDEADKEY)



#define setobj(L,obj1,obj2) \
	{ NameDef(TValue) *io1=(obj1); *io1 = *(obj2); \
	  (void)L; checkliveness(G(L),io1); }


/*
** different types of assignments, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue




/*
** {======================================================
** types and prototypes
** =======================================================
*/


union NameDef(Value) {
  NameDef(GCObject) *gc;    /* collectable objects */
  void *p;         /* light userdata */
  int b;           /* booleans */
  NameDef(lua_CFunction) f; /* light C functions */
  NameDef(lua_Integer) i;   /* integer numbers */
  NameDef(lua_Number) n;    /* float numbers */
};


struct NameDef(lua_TValue) {
  TValuefields;
};


typedef NameDef(TValue) *NameDef(StkId);  /* index to stack elements */




/*
** Header for string value; string bytes follow the end of this structure
** (aligned according to 'UTString'; see next).
*/
typedef struct NameDef(TString) {
  CommonHeader;
  NameDef(lu_byte) extra;  /* reserved words for short strings; "has hash" for longs */
  NameDef(lu_byte) shrlen;  /* length for short strings */
  unsigned int hash;
  union {
    size_t lnglen;  /* length for long strings */
    struct NameDef(TString) *hnext;  /* linked list for hash table */
  } u;
} NameDef(TString);


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union NameDef(UTString) {
  NameDef(L_Umaxalign) dummy;  /* ensures maximum alignment for strings */
  NameDef(TString) tsv;
} NameDef(UTString);


/*
** Get the actual string (array of bytes) from a 'TString'.
** (Access to 'extra' ensures that value is really a 'TString'.)
*/
#define getaddrstr(ts)	(cast(char *, (ts)) + sizeof(NameDef(UTString)))
#define getstr(ts)  \
  check_exp(sizeof((ts)->extra), cast(const char*, getaddrstr(ts)))

/* get the actual string (array of bytes) from a Lua value */
#define svalue(o)       getstr(tsvalue(o))

/* get string length from 'TString *s' */
#define tsslen(s)	((s)->tt == LUA_TSHRSTR ? (s)->shrlen : (s)->u.lnglen)

/* get string length from 'TValue *o' */
#define vslen(o)	tsslen(tsvalue(o))


/*
** Header for userdata; memory area follows the end of this structure
** (aligned according to 'UUdata'; see next).
*/
typedef struct NameDef(Udata) {
  CommonHeader;
  NameDef(lu_byte) ttuv_;  /* user value's tag */
  struct NameDef(Table) *metatable;
  size_t len;  /* number of bytes */
  union NameDef(Value) user_;  /* user value */
} NameDef(Udata);


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union NameDef(UUdata) {
  NameDef(L_Umaxalign) dummy;  /* ensures maximum alignment for 'local' udata */
  NameDef(Udata) uv;
} NameDef(UUdata);


/*
**  Get the address of memory block inside 'Udata'.
** (Access to 'ttuv_' ensures that value is really a 'Udata'.)
*/
#define getudatamem(u)  \
  check_exp(sizeof((u)->ttuv_), (cast(char*, (u)) + sizeof(NameDef(UUdata))))

#define setuservalue(L,u,o) \
	{ const NameDef(TValue) *io=(o); NameDef(Udata) *iu = (u); \
	  iu->user_ = io->value_; iu->ttuv_ = rttype(io); \
	  checkliveness(G(L),io); }


#define getuservalue(L,u,o) \
	{ NameDef(TValue) *io=(o); const NameDef(Udata) *iu = (u); \
	  io->value_ = iu->user_; settt_(io, iu->ttuv_); \
	  checkliveness(G(L),io); }


/*
** Description of an upvalue for function prototypes
*/
typedef struct NameDef(Upvaldesc) {
  NameDef(TString) *name;  /* upvalue name (for debug information) */
  NameDef(lu_byte) instack;  /* whether it is in stack (register) */
  NameDef(lu_byte) idx;  /* index of upvalue (in stack or in outer function's list) */
} NameDef(Upvaldesc);


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
typedef struct NameDef(LocVar) {
  NameDef(TString) *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} NameDef(LocVar);


/*
** Function Prototypes
*/
typedef struct NameDef(Proto) {
  CommonHeader;
  NameDef(lu_byte) numparams;  /* number of fixed parameters */
  NameDef(lu_byte) is_vararg;
  NameDef(lu_byte) maxstacksize;  /* number of registers needed by this function */
  int sizeupvalues;  /* size of 'upvalues' */
  int sizek;  /* size of 'k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of 'p' */
  int sizelocvars;
  int linedefined;
  int lastlinedefined;
  NameDef(TValue) *k;  /* constants used by the function */
  NameDef(Instruction) *code;  /* opcodes */
  struct NameDef(Proto) **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines (debug information) */
  NameDef(LocVar) *locvars;  /* information about local variables (debug information) */
  NameDef(Upvaldesc) *upvalues;  /* upvalue information */
  struct NameDef(LClosure) *cache;  /* last-created closure with this prototype */
  NameDef(TString)  *source;  /* used for debug information */
  NameDef(GCObject) *gclist;
} NameDef(Proto);



/*
** Lua Upvalues
*/
typedef struct NameDef(UpVal) NameDef(UpVal);


/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; NameDef(lu_byte) nupvalues; NameDef(GCObject) *gclist

typedef struct NameDef(CClosure) {
  ClosureHeader;
  NameDef(lua_CFunction) f;
  NameDef(TValue) upvalue[1];  /* list of upvalues */
} NameDef(CClosure);


typedef struct NameDef(LClosure) {
  ClosureHeader;
  struct NameDef(Proto) *p;
  NameDef(UpVal) *upvals[1];  /* list of upvalues */
} NameDef(LClosure);


typedef union NameDef(Closure) {
  NameDef(CClosure) c;
  NameDef(LClosure) l;
} NameDef(Closure);


#define isLfunction(o)	ttisLclosure(o)

#define getproto(o)	(clLvalue(o)->p)


/*
** Tables
*/

typedef union NameDef(TKey) {
  struct {
    TValuefields;
    int next;  /* for chaining (offset for next node) */
  } nk;
  NameDef(TValue) tvk;
} NameDef(TKey);


/* copy a value into a key without messing up field 'next' */
#define setnodekey(L,key,obj) \
	{ NameDef(TKey) *k_=(key); const NameDef(TValue) *io_=(obj); \
	  k_->nk.value_ = io_->value_; k_->nk.tt_ = io_->tt_; \
	  (void)L; checkliveness(G(L),io_); }


typedef struct NameDef(Node) {
  NameDef(TValue) i_val;
  NameDef(TKey) i_key;
} NameDef(Node);


typedef struct NameDef(Table) {
  CommonHeader;
  NameDef(lu_byte) flags;  /* 1<<p means tagmethod(p) is not present */
  NameDef(lu_byte) lsizenode;  /* log2 of size of 'node' array */
  unsigned int sizearray;  /* size of 'array' array */
  NameDef(TValue) *array;  /* array part */
  NameDef(Node) *node;
  NameDef(Node) *lastfree;  /* any free position is before this position */
  struct NameDef(Table) *metatable;
  NameDef(GCObject) *gclist;
} NameDef(Table);



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


/*
** (address of) a fixed nil value
*/
#define luaO_nilobject		(&NameDef(luaO_nilobject_))


LUAI_DDEC const NameDef(TValue) NameDef(luaO_nilobject_);

/* size of buffer for 'luaO_utf8esc' function */
#define UTF8BUFFSZ	8

LUAI_FUNC int NameDef(luaO_int2fb) (unsigned int x);
LUAI_FUNC int NameDef(luaO_fb2int) (int x);
LUAI_FUNC int NameDef(luaO_utf8esc) (char *buff, unsigned long x);
LUAI_FUNC int NameDef(luaO_ceillog2) (unsigned int x);
LUAI_FUNC void NameDef(luaO_arith) (NameDef(lua_State) *L, int op, const NameDef(TValue) *p1,
                           const NameDef(TValue) *p2, NameDef(TValue) *res);
LUAI_FUNC size_t NameDef(luaO_str2num) (const char *s, NameDef(TValue) *o);
LUAI_FUNC int NameDef(luaO_hexavalue) (int c);
LUAI_FUNC void NameDef(luaO_tostring) (NameDef(lua_State) *L, NameDef(StkId) obj);
LUAI_FUNC const char *NameDef(luaO_pushvfstring) (NameDef(lua_State) *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *NameDef(luaO_pushfstring) (NameDef(lua_State) *L, const char *fmt, ...);
LUAI_FUNC void NameDef(luaO_chunkid) (char *out, const char *source, size_t len);


#endif

