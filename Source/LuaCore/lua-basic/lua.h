/*
** $Id: lua.h,v 1.331 2016/05/30 15:53:28 roberto Exp $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (http://www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>


#include "luaconf.h"


#define LUA_VERSION_MAJOR	"5"
#define LUA_VERSION_MINOR	"3"
#define LUA_VERSION_NUM		503
#define LUA_VERSION_RELEASE	"3"

#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE
#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2016 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


/* mark for precompiled code ('<esc>Lua') */
#define LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (-LUAI_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
*/
#define LUA_REGISTRYINDEX	(-LUAI_MAXSTACK - 1000)
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRGCMM	5
#define LUA_ERRERR	6


typedef struct NameDef(lua_State) NameDef(lua_State);


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTAGS		9



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/* predefined values in the registry */
#define LUA_RIDX_MAINTHREAD	1
#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_LAST		LUA_RIDX_GLOBALS


/* type of numbers in Lua */
typedef LUA_NUMBER NameDef(lua_Number);


/* type for integer functions */
typedef LUA_INTEGER NameDef(lua_Integer);

/* unsigned integer type */
typedef LUA_UNSIGNED NameDef(lua_Unsigned);

/* type for continuation-function contexts */
typedef LUA_KCONTEXT NameDef(lua_KContext);


/*
** Type for C functions registered with Lua
*/
typedef int (*NameDef(lua_CFunction)) (NameDef(lua_State) *L);

/*
** Type for continuation functions
*/
typedef int (*NameDef(lua_KFunction)) (NameDef(lua_State) *L, int status, NameDef(lua_KContext) ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*NameDef(lua_Reader)) (NameDef(lua_State) *L, void *ud, size_t *sz);

typedef int (*NameDef(lua_Writer)) (NameDef(lua_State) *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
typedef void * (*NameDef(lua_Alloc)) (void *ud, void *ptr, size_t osize, size_t nsize);



/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string
*/
extern const char NameDef(lua_ident)[];


/*
** state manipulation
*/
LUA_API NameDef(lua_State) *(NameDef(lua_newstate)) (NameDef(lua_Alloc) f, void *ud);
LUA_API void       (NameDef(lua_close)) (NameDef(lua_State) *L);
LUA_API NameDef(lua_State) *(NameDef(lua_newthread)) (NameDef(lua_State) *L);

LUA_API NameDef(lua_CFunction) (NameDef(lua_atpanic)) (NameDef(lua_State) *L, NameDef(lua_CFunction) panicf);


LUA_API const NameDef(lua_Number) *(NameDef(lua_version)) (NameDef(lua_State) *L);


/*
** basic stack manipulation
*/
LUA_API int   (NameDef(lua_absindex)) (NameDef(lua_State) *L, int idx);
LUA_API int   (NameDef(lua_gettop)) (NameDef(lua_State) *L);
LUA_API void  (NameDef(lua_settop)) (NameDef(lua_State) *L, int idx);
LUA_API void  (NameDef(lua_pushvalue)) (NameDef(lua_State) *L, int idx);
LUA_API void  (NameDef(lua_rotate)) (NameDef(lua_State) *L, int idx, int n);
LUA_API void  (NameDef(lua_copy)) (NameDef(lua_State) *L, int fromidx, int toidx);
LUA_API int   (NameDef(lua_checkstack)) (NameDef(lua_State) *L, int n);

LUA_API void  (NameDef(lua_xmove)) (NameDef(lua_State) *from, NameDef(lua_State) *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (NameDef(lua_isnumber)) (NameDef(lua_State) *L, int idx);
LUA_API int             (NameDef(lua_isstring)) (NameDef(lua_State) *L, int idx);
LUA_API int             (NameDef(lua_iscfunction)) (NameDef(lua_State) *L, int idx);
LUA_API int             (NameDef(lua_isinteger)) (NameDef(lua_State) *L, int idx);
LUA_API int             (NameDef(lua_isuserdata)) (NameDef(lua_State) *L, int idx);
LUA_API int             (NameDef(lua_type)) (NameDef(lua_State) *L, int idx);
LUA_API const char     *(NameDef(lua_typename)) (NameDef(lua_State) *L, int tp);

LUA_API NameDef(lua_Number)      (NameDef(lua_tonumberx)) (NameDef(lua_State) *L, int idx, int *isnum);
LUA_API NameDef(lua_Integer)     (NameDef(lua_tointegerx)) (NameDef(lua_State) *L, int idx, int *isnum);
LUA_API int             (NameDef(lua_toboolean)) (NameDef(lua_State) *L, int idx);
LUA_API const char     *(NameDef(lua_tolstring)) (NameDef(lua_State) *L, int idx, size_t *len);
LUA_API size_t          (NameDef(lua_rawlen)) (NameDef(lua_State) *L, int idx);
LUA_API NameDef(lua_CFunction)   (NameDef(lua_tocfunction)) (NameDef(lua_State) *L, int idx);
LUA_API void	       *(NameDef(lua_touserdata)) (NameDef(lua_State) *L, int idx);
LUA_API NameDef(lua_State)      *(NameDef(lua_tothread)) (NameDef(lua_State) *L, int idx);
LUA_API const void     *(NameDef(lua_topointer)) (NameDef(lua_State) *L, int idx);


/*
** Comparison and arithmetic functions
*/

#define LUA_OPADD	0	/* ORDER TM, ORDER OP */
#define LUA_OPSUB	1
#define LUA_OPMUL	2
#define LUA_OPMOD	3
#define LUA_OPPOW	4
#define LUA_OPDIV	5
#define LUA_OPIDIV	6
#define LUA_OPBAND	7
#define LUA_OPBOR	8
#define LUA_OPBXOR	9
#define LUA_OPSHL	10
#define LUA_OPSHR	11
#define LUA_OPUNM	12
#define LUA_OPBNOT	13

LUA_API void  (NameDef(lua_arith)) (NameDef(lua_State) *L, int op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API int   (NameDef(lua_rawequal)) (NameDef(lua_State) *L, int idx1, int idx2);
LUA_API int   (NameDef(lua_compare)) (NameDef(lua_State) *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack)
*/
LUA_API void        (NameDef(lua_pushnil)) (NameDef(lua_State) *L);
LUA_API void        (NameDef(lua_pushnumber)) (NameDef(lua_State) *L, NameDef(lua_Number) n);
LUA_API void        (NameDef(lua_pushinteger)) (NameDef(lua_State) *L, NameDef(lua_Integer) n);
LUA_API const char *(NameDef(lua_pushlstring)) (NameDef(lua_State) *L, const char *s, size_t len);
LUA_API const char *(NameDef(lua_pushstring)) (NameDef(lua_State) *L, const char *s);
LUA_API const char *(NameDef(lua_pushvfstring)) (NameDef(lua_State) *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(NameDef(lua_pushfstring)) (NameDef(lua_State) *L, const char *fmt, ...);
LUA_API void  (NameDef(lua_pushcclosure)) (NameDef(lua_State) *L, NameDef(lua_CFunction) fn, int n);
LUA_API void  (NameDef(lua_pushboolean)) (NameDef(lua_State) *L, int b);
LUA_API void  (NameDef(lua_pushlightuserdata)) (NameDef(lua_State) *L, void *p);
LUA_API int   (NameDef(lua_pushthread)) (NameDef(lua_State) *L);


/*
** get functions (Lua -> stack)
*/
LUA_API int (NameDef(lua_getglobal)) (NameDef(lua_State) *L, const char *name);
LUA_API int (NameDef(lua_gettable)) (NameDef(lua_State) *L, int idx);
LUA_API int (NameDef(lua_getfield)) (NameDef(lua_State) *L, int idx, const char *k);
LUA_API int (NameDef(lua_geti)) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n);
LUA_API int (NameDef(lua_rawget)) (NameDef(lua_State) *L, int idx);
LUA_API int (NameDef(lua_rawgeti)) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n);
LUA_API int (NameDef(lua_rawgetp)) (NameDef(lua_State) *L, int idx, const void *p);

LUA_API void  (NameDef(lua_createtable)) (NameDef(lua_State) *L, int narr, int nrec);
LUA_API void *(NameDef(lua_newuserdata)) (NameDef(lua_State) *L, size_t sz);
LUA_API int   (NameDef(lua_getmetatable)) (NameDef(lua_State) *L, int objindex);
LUA_API int  (NameDef(lua_getuservalue)) (NameDef(lua_State) *L, int idx);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (NameDef(lua_setglobal)) (NameDef(lua_State) *L, const char *name);
LUA_API void  (NameDef(lua_settable)) (NameDef(lua_State) *L, int idx);
LUA_API void  (NameDef(lua_setfield)) (NameDef(lua_State) *L, int idx, const char *k);
LUA_API void  (NameDef(lua_seti)) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n);
LUA_API void  (NameDef(lua_rawset)) (NameDef(lua_State) *L, int idx);
LUA_API void  (NameDef(lua_rawseti)) (NameDef(lua_State) *L, int idx, NameDef(lua_Integer) n);
LUA_API void  (NameDef(lua_rawsetp)) (NameDef(lua_State) *L, int idx, const void *p);
LUA_API int   (NameDef(lua_setmetatable)) (NameDef(lua_State) *L, int objindex);
LUA_API void  (NameDef(lua_setuservalue)) (NameDef(lua_State) *L, int idx);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API void  (NameDef(lua_callk)) (NameDef(lua_State) *L, int nargs, int nresults,
                           NameDef(lua_KContext) ctx, NameDef(lua_KFunction) k);
#define lua_call(L,n,r)		NameDef(lua_callk)(L, (n), (r), 0, NULL)

LUA_API int   (NameDef(lua_pcallk)) (NameDef(lua_State) *L, int nargs, int nresults, int errfunc,
                            NameDef(lua_KContext) ctx, NameDef(lua_KFunction) k);
#define lua_pcall(L,n,r,f)	NameDef(lua_pcallk)(L, (n), (r), (f), 0, NULL)

LUA_API int   (NameDef(lua_load)) (NameDef(lua_State) *L, NameDef(lua_Reader) reader, void *dt,
                          const char *chunkname, const char *mode);

LUA_API int (NameDef(lua_dump)) (NameDef(lua_State) *L, NameDef(lua_Writer) writer, void *data, int strip);


/*
** coroutine functions
*/
LUA_API int  (NameDef(lua_yieldk))     (NameDef(lua_State) *L, int nresults, NameDef(lua_KContext) ctx,
                               NameDef(lua_KFunction) k);
LUA_API int  (NameDef(lua_resume))     (NameDef(lua_State) *L, NameDef(lua_State) *from, int narg);
LUA_API int  (NameDef(lua_status))     (NameDef(lua_State) *L);
LUA_API int (NameDef(lua_isyieldable)) (NameDef(lua_State) *L);

#define lua_yield(L,n)		NameDef(lua_yieldk)(L, (n), 0, NULL)


/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7
#define LUA_GCISRUNNING		9

LUA_API int (NameDef(lua_gc)) (NameDef(lua_State) *L, int what, int data);


/*
** miscellaneous functions
*/

LUA_API int   (NameDef(lua_error)) (NameDef(lua_State) *L);

LUA_API int   (NameDef(lua_next)) (NameDef(lua_State) *L, int idx);

LUA_API void  (NameDef(lua_concat)) (NameDef(lua_State) *L, int n);
LUA_API void  (NameDef(lua_len))    (NameDef(lua_State) *L, int idx);

LUA_API size_t   (NameDef(lua_stringtonumber)) (NameDef(lua_State) *L, const char *s);

LUA_API NameDef(lua_Alloc) (NameDef(lua_getallocf)) (NameDef(lua_State) *L, void **ud);
LUA_API void      (NameDef(lua_setallocf)) (NameDef(lua_State) *L, NameDef(lua_Alloc) f, void *ud);



/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	NameDef(lua_tonumberx)(L,(i),NULL)
#define lua_tointeger(L,i)	NameDef(lua_tointegerx)(L,(i),NULL)

#define lua_pop(L,n)		NameDef(lua_settop)(L, -(n)-1)

#define lua_newtable(L)		NameDef(lua_createtable)(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), NameDef(lua_setglobal)(L, (n)))

#define lua_pushcfunction(L,f)	NameDef(lua_pushcclosure)(L, (f), 0)

#define lua_isfunction(L,n)	(NameDef(lua_type)(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(NameDef(lua_type)(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(NameDef(lua_type)(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(NameDef(lua_type)(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(NameDef(lua_type)(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(NameDef(lua_type)(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(NameDef(lua_type)(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(NameDef(lua_type)(L, (n)) <= 0)

#define lua_pushliteral(L, s)	NameDef(lua_pushstring)(L, "" s)

#define lua_pushglobaltable(L)  \
	((void)NameDef(lua_rawgeti)(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	NameDef(lua_tolstring)(L, (i), NULL)


#define lua_insert(L,idx)	NameDef(lua_rotate)(L, (idx), 1)

#define lua_remove(L,idx)	(NameDef(lua_rotate)(L, (idx), -1), lua_pop(L, 1))

#define lua_replace(L,idx)	(NameDef(lua_copy)(L, -1, (idx)), lua_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros for unsigned conversions
** ===============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define lua_pushunsigned(L,n)	NameDef(lua_pushinteger)(L, (NameDef(lua_Integer))(n))
#define lua_tounsignedx(L,i,is)	((NameDef(lua_Unsigned))NameDef(lua_tointegerx)(L,i,is))
#define lua_tounsigned(L,i)	lua_tounsignedx(L,(i),NULL)

#endif
/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct NameDef(lua_Debug) NameDef(lua_Debug);  /* activation record */


/* Functions to be called by the debugger in specific events */
typedef void (*NameDef(lua_Hook)) (NameDef(lua_State) *L, NameDef(lua_Debug) *ar);


LUA_API int (NameDef(lua_getstack)) (NameDef(lua_State) *L, int level, NameDef(lua_Debug) *ar);
LUA_API int (NameDef(lua_getinfo)) (NameDef(lua_State) *L, const char *what, NameDef(lua_Debug) *ar);
LUA_API const char *(NameDef(lua_getlocal)) (NameDef(lua_State) *L, const NameDef(lua_Debug) *ar, int n);
LUA_API const char *(NameDef(lua_setlocal)) (NameDef(lua_State) *L, const NameDef(lua_Debug) *ar, int n);
LUA_API const char *(NameDef(lua_getupvalue)) (NameDef(lua_State) *L, int funcindex, int n);
LUA_API const char *(NameDef(lua_setupvalue)) (NameDef(lua_State) *L, int funcindex, int n);

LUA_API void *(NameDef(lua_upvalueid)) (NameDef(lua_State) *L, int fidx, int n);
LUA_API void  (NameDef(lua_upvaluejoin)) (NameDef(lua_State) *L, int fidx1, int n1,
                                               int fidx2, int n2);

LUA_API void (NameDef(lua_sethook)) (NameDef(lua_State) *L, NameDef(lua_Hook) func, int mask, int count);
LUA_API NameDef(lua_Hook) (NameDef(lua_gethook)) (NameDef(lua_State) *L);
LUA_API int (NameDef(lua_gethookmask)) (NameDef(lua_State) *L);
LUA_API int (NameDef(lua_gethookcount)) (NameDef(lua_State) *L);


struct NameDef(lua_Debug) {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  const char *source;	/* (S) */
  int currentline;	/* (l) */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  unsigned char nups;	/* (u) number of upvalues */
  unsigned char nparams;/* (u) number of parameters */
  char isvararg;        /* (u) */
  char istailcall;	/* (t) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  struct NameDef(CallInfo) *i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 1994-2016 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
