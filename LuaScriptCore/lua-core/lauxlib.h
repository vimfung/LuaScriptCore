/*
** $Id: lauxlib.h,v 1.128 2014/10/29 16:11:17 roberto Exp $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h

#include "LuaDefine.h"

#include <stddef.h>
#include <stdio.h>

#include "lua.h"



/* extra error code for 'luaL_load' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


typedef struct NameDef(luaL_Reg) {
  const char *name;
  NameDef(lua_CFunction) func;
} NameDef(luaL_Reg);


#define LUAL_NUMSIZES	(sizeof(NameDef(lua_Integer))*16 + sizeof(NameDef(lua_Number)))

LUALIB_API void (NameDef(luaL_checkversion_)) (NameDef(lua_State) *L, NameDef(lua_Number) ver, size_t sz);
#define luaL_checkversion(L)  \
	  NameDef(luaL_checkversion_)(L, LUA_VERSION_NUM, LUAL_NUMSIZES)

LUALIB_API int (NameDef(luaL_getmetafield)) (NameDef(lua_State) *L, int obj, const char *e);
LUALIB_API int (NameDef(luaL_callmeta)) (NameDef(lua_State) *L, int obj, const char *e);
LUALIB_API const char *(NameDef(luaL_tolstring)) (NameDef(lua_State) *L, int idx, size_t *len);
LUALIB_API int (NameDef(luaL_argerror)) (NameDef(lua_State) *L, int arg, const char *extramsg);
LUALIB_API const char *(NameDef(luaL_checklstring)) (NameDef(lua_State) *L, int arg,
                                                          size_t *l);
LUALIB_API const char *(NameDef(luaL_optlstring)) (NameDef(lua_State) *L, int arg,
                                          const char *def, size_t *l);
LUALIB_API NameDef(lua_Number) (NameDef(luaL_checknumber)) (NameDef(lua_State) *L, int arg);
LUALIB_API NameDef(lua_Number) (NameDef(luaL_optnumber)) (NameDef(lua_State) *L, int arg, NameDef(lua_Number) def);

LUALIB_API NameDef(lua_Integer) (NameDef(luaL_checkinteger)) (NameDef(lua_State) *L, int arg);
LUALIB_API NameDef(lua_Integer) (NameDef(luaL_optinteger)) (NameDef(lua_State) *L, int arg,
                                          NameDef(lua_Integer) def);

LUALIB_API void (NameDef(luaL_checkstack)) (NameDef(lua_State) *L, int sz, const char *msg);
LUALIB_API void (NameDef(luaL_checktype)) (NameDef(lua_State) *L, int arg, int t);
LUALIB_API void (NameDef(luaL_checkany)) (NameDef(lua_State) *L, int arg);

LUALIB_API int   (NameDef(luaL_newmetatable)) (NameDef(lua_State) *L, const char *tname);
LUALIB_API void  (NameDef(luaL_setmetatable)) (NameDef(lua_State) *L, const char *tname);
LUALIB_API void *(NameDef(luaL_testudata)) (NameDef(lua_State) *L, int ud, const char *tname);
LUALIB_API void *(NameDef(luaL_checkudata)) (NameDef(lua_State) *L, int ud, const char *tname);

LUALIB_API void (NameDef(luaL_where)) (NameDef(lua_State) *L, int lvl);
LUALIB_API int (NameDef(luaL_error)) (NameDef(lua_State) *L, const char *fmt, ...);

LUALIB_API int (NameDef(luaL_checkoption)) (NameDef(lua_State) *L, int arg, const char *def,
                                   const char *const lst[]);

LUALIB_API int (NameDef(luaL_fileresult)) (NameDef(lua_State) *L, int stat, const char *fname);
LUALIB_API int (NameDef(luaL_execresult)) (NameDef(lua_State) *L, int stat);

/* pre-defined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API int (NameDef(luaL_ref)) (NameDef(lua_State) *L, int t);
LUALIB_API void (NameDef(luaL_unref)) (NameDef(lua_State) *L, int t, int ref);

LUALIB_API int (NameDef(luaL_loadfilex)) (NameDef(lua_State) *L, const char *filename,
                                               const char *mode);

#define luaL_loadfile(L,f)	NameDef(luaL_loadfilex)(L,f,NULL)

LUALIB_API int (NameDef(luaL_loadbufferx)) (NameDef(lua_State) *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
LUALIB_API int (NameDef(luaL_loadstring)) (NameDef(lua_State) *L, const char *s);

LUALIB_API NameDef(lua_State) *(NameDef(luaL_newstate)) (void);

LUALIB_API NameDef(lua_Integer) (NameDef(luaL_len)) (NameDef(lua_State) *L, int idx);

LUALIB_API const char *(NameDef(luaL_gsub)) (NameDef(lua_State) *L, const char *s, const char *p,
                                                  const char *r);

LUALIB_API void (NameDef(luaL_setfuncs)) (NameDef(lua_State) *L, const NameDef(luaL_Reg) *l, int nup);

LUALIB_API int (NameDef(luaL_getsubtable)) (NameDef(lua_State) *L, int idx, const char *fname);

LUALIB_API void (NameDef(luaL_traceback)) (NameDef(lua_State )*L, NameDef(lua_State) *L1,
                                  const char *msg, int level);

LUALIB_API void (NameDef(luaL_requiref)) (NameDef(lua_State) *L, const char *modname,
                                 NameDef(lua_CFunction) openf, int glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  NameDef(lua_createtable)(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), NameDef(luaL_setfuncs)(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
		((void)((cond) || NameDef(luaL_argerror)(L, (arg), (extramsg))))
#define luaL_checkstring(L,n)	(NameDef(luaL_checklstring)(L, (n), NULL))
#define luaL_optstring(L,n,d)	(NameDef(luaL_optlstring)(L, (n), (d), NULL))

#define luaL_typename(L,i)	NameDef(lua_typename)(L, NameDef(lua_type)(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(NameDef(lua_getfield)(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	NameDef(luaL_loadbufferx)(L,s,sz,n,NULL)


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

typedef struct NameDef(luaL_Buffer) {
  char *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  NameDef(lua_State) *L;
  char initb[LUAL_BUFFERSIZE];  /* initial buffer */
} NameDef(luaL_Buffer);


#define luaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || NameDef(luaL_prepbuffsize)((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

LUALIB_API void (NameDef(luaL_buffinit)) (NameDef(lua_State) *L, NameDef(luaL_Buffer) *B);
LUALIB_API char *(NameDef(luaL_prepbuffsize)) (NameDef(luaL_Buffer) *B, size_t sz);
LUALIB_API void (NameDef(luaL_addlstring)) (NameDef(luaL_Buffer) *B, const char *s, size_t l);
LUALIB_API void (NameDef(luaL_addstring)) (NameDef(luaL_Buffer) *B, const char *s);
LUALIB_API void (NameDef(luaL_addvalue)) (NameDef(luaL_Buffer) *B);
LUALIB_API void (NameDef(luaL_pushresult)) (NameDef(luaL_Buffer) *B);
LUALIB_API void (NameDef(luaL_pushresultsize)) (NameDef(luaL_Buffer) *B, size_t sz);
LUALIB_API char *(NameDef(luaL_buffinitsize)) (NameDef(lua_State) *L, NameDef(luaL_Buffer) *B, size_t sz);

#define luaL_prepbuffer(B)	NameDef(luaL_prepbuffsize)(B, LUAL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'luaL_Stream' (it may contain other fields
** after that initial structure).
*/

#define LUA_FILEHANDLE          "FILE*"


typedef struct NameDef(luaL_Stream) {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  NameDef(lua_CFunction) closef;  /* to close stream (NULL for closed streams) */
} NameDef(luaL_Stream);

/* }====================================================== */



/* compatibility with old module system */
#if defined(LUA_COMPAT_MODULE)

LUALIB_API void (NameDef(luaL_pushmodule)) (lua_State *L, const char *modname,
                                   int sizehint);
LUALIB_API void (NameDef(luaL_openlib)) (lua_State *L, const char *libname,
                                const luaL_Reg *l, int nup);

#define luaL_register(L,n,l)	(luaL_openlib(L,(n),(l),0))

#endif


/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#if !defined(lua_writestring)
#define lua_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output */
#if !defined(lua_writeline)
#define lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#if !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


