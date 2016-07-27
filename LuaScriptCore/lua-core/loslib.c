/*
** $Id: loslib.c,v 1.64 2016/04/18 13:06:55 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#include "lprefix.h"


#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#if !defined(LUA_STRFTIMEOPTIONS)	/* { */

/* options for ANSI C 89 */
#define L_STRFTIMEC89		"aAbBcdHIjmMpSUwWxXyYZ%"

/* options for ISO C 99 and POSIX */
#define L_STRFTIMEC99 "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
	"||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"

/* options for Windows */
#define L_STRFTIMEWIN "aAbBcdHIjmMpSUwWxXyYzZ%" \
	"||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"

#if defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS	L_STRFTIMEWIN
#elif defined(LUA_USE_C89)
#define LUA_STRFTIMEOPTIONS	L_STRFTIMEC89
#else  /* C99 specification */
#define LUA_STRFTIMEOPTIONS	L_STRFTIMEC99
#endif

#endif					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

#if !defined(l_time_t)		/* { */
/*
** type to represent time_t in Lua
*/
#define l_timet			NameDef(lua_Integer)
#define l_pushtime(L,t)		NameDef(lua_pushinteger)(L,(NameDef(lua_Integer))(t))

static time_t l_checktime (NameDef(lua_State) *L, int arg) {
  NameDef(lua_Integer) t = NameDef(luaL_checkinteger)(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  return (time_t)t;
}

#endif				/* } */


#if !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#if defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#else				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((void)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)  	((void)(r)->tm_sec, localtime(t))

#endif				/* } */

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#if !defined(lua_tmpnam)	/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#if !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        if (e != -1) close(e); \
        e = (e == -1); }

#else				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == NULL); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */




static int os_execute (NameDef(lua_State) *L) {
  const char *cmd = luaL_optstring(L, 1, NULL);
  int stat = system(cmd);
  if (cmd != NULL)
    return NameDef(luaL_execresult)(L, stat);
  else {
    NameDef(lua_pushboolean)(L, stat);  /* true if there is a shell */
    return 1;
  }
}


static int os_remove (NameDef(lua_State) *L) {
  const char *filename = luaL_checkstring(L, 1);
  return NameDef(luaL_fileresult)(L, remove(filename) == 0, filename);
}


static int os_rename (NameDef(lua_State) *L) {
  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);
  return NameDef(luaL_fileresult)(L, rename(fromname, toname) == 0, NULL);
}


static int os_tmpname (NameDef(lua_State) *L) {
  char buff[LUA_TMPNAMBUFSIZE];
  int err;
  lua_tmpnam(buff, err);
  if (err)
    return NameDef(luaL_error)(L, "unable to generate a unique filename");
  NameDef(lua_pushstring)(L, buff);
  return 1;
}


static int os_getenv (NameDef(lua_State) *L) {
  NameDef(lua_pushstring)(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  return 1;
}


static int os_clock (NameDef(lua_State) *L) {
  NameDef(lua_pushnumber)(L, ((NameDef(lua_Number))clock())/(NameDef(lua_Number))CLOCKS_PER_SEC);
  return 1;
}


/*
** {======================================================
** Time/Date operations
** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
**   wday=%w+1, yday=%j, isdst=? }
** =======================================================
*/

static void setfield (NameDef(lua_State) *L, const char *key, int value) {
  NameDef(lua_pushinteger)(L, value);
  NameDef(lua_setfield)(L, -2, key);
}

static void setboolfield (NameDef(lua_State) *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  NameDef(lua_pushboolean)(L, value);
  NameDef(lua_setfield)(L, -2, key);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
static void setallfields (NameDef(lua_State) *L, struct tm *stm) {
  setfield(L, "sec", stm->tm_sec);
  setfield(L, "min", stm->tm_min);
  setfield(L, "hour", stm->tm_hour);
  setfield(L, "day", stm->tm_mday);
  setfield(L, "month", stm->tm_mon + 1);
  setfield(L, "year", stm->tm_year + 1900);
  setfield(L, "wday", stm->tm_wday + 1);
  setfield(L, "yday", stm->tm_yday + 1);
  setboolfield(L, "isdst", stm->tm_isdst);
}


static int getboolfield (NameDef(lua_State) *L, const char *key) {
  int res;
  res = (NameDef(lua_getfield)(L, -1, key) == LUA_TNIL) ? -1 : NameDef(lua_toboolean)(L, -1);
  lua_pop(L, 1);
  return res;
}


/* maximum value for date fields (to avoid arithmetic overflows with 'int') */
#if !defined(L_MAXDATEFIELD)
#define L_MAXDATEFIELD	(INT_MAX / 2)
#endif

static int getfield (NameDef(lua_State) *L, const char *key, int d, int delta) {
  int isnum;
  int t = NameDef(lua_getfield)(L, -1, key);  /* get field and its type */
  NameDef(lua_Integer) res = NameDef(lua_tointegerx)(L, -1, &isnum);
  if (!isnum) {  /* field is not an integer? */
    if (t != LUA_TNIL)  /* some other value? */
      return NameDef(luaL_error)(L, "field '%s' is not an integer", key);
    else if (d < 0)  /* absent field; no default? */
      return NameDef(luaL_error)(L, "field '%s' missing in date table", key);
    res = d;
  }
  else {
    if (!(-L_MAXDATEFIELD <= res && res <= L_MAXDATEFIELD))
      return NameDef(luaL_error)(L, "field '%s' is out-of-bound", key);
    res -= delta;
  }
  lua_pop(L, 1);
  return (int)res;
}


static const char *checkoption (NameDef(lua_State) *L, const char *conv, char *buff) {
  const char *option;
  int oplen = 1;
  for (option = LUA_STRFTIMEOPTIONS; *option != '\0'; option += oplen) {
    if (*option == '|')  /* next block? */
      oplen++;  /* next length */
    else if (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      return conv + oplen;  /* return next item */
    }
  }
  NameDef(luaL_argerror)(L, 1,
    NameDef(lua_pushfstring)(L, "invalid conversion specifier '%%%s'", conv));
  return conv;  /* to avoid warnings */
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


static int os_date (NameDef(lua_State) *L) {
  const char *s = luaL_optstring(L, 1, "%c");
  time_t t = luaL_opt(L, l_checktime, 2, time(NULL));
  struct tm tmr, *stm;
  if (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  else
    stm = l_localtime(&t, &tmr);
  if (stm == NULL)  /* invalid date? */
    NameDef(luaL_error)(L, "time result cannot be represented in this installation");
  if (strcmp(s, "*t") == 0) {
    NameDef(lua_createtable)(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  else {
    char cc[4];  /* buffer for individual conversion specifiers */
    NameDef(luaL_Buffer) b;
    cc[0] = '%';
    NameDef(luaL_buffinit)(L, &b);
    while (*s) {
      if (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      else {
        size_t reslen;
        char *buff = NameDef(luaL_prepbuffsize)(&b, SIZETIMEFMT);
        s = checkoption(L, s + 1, cc + 1);  /* copy specifier to 'cc' */
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    NameDef(luaL_pushresult)(&b);
  }
  return 1;
}


static int os_time (NameDef(lua_State) *L) {
  time_t t;
  if (lua_isnoneornil(L, 1))  /* called without args? */
    t = time(NULL);  /* get current time */
  else {
    struct tm ts;
    NameDef(luaL_checktype)(L, 1, LUA_TTABLE);
    NameDef(lua_settop)(L, 1);  /* make sure table is at the top */
    ts.tm_sec = getfield(L, "sec", 0, 0);
    ts.tm_min = getfield(L, "min", 0, 0);
    ts.tm_hour = getfield(L, "hour", 12, 0);
    ts.tm_mday = getfield(L, "day", -1, 0);
    ts.tm_mon = getfield(L, "month", -1, 1);
    ts.tm_year = getfield(L, "year", -1, 1900);
    ts.tm_isdst = getboolfield(L, "isdst");
    t = mktime(&ts);
    setallfields(L, &ts);  /* update fields with normalized values */
  }
  if (t != (time_t)(l_timet)t || t == (time_t)(-1))
    NameDef(luaL_error)(L, "time result cannot be represented in this installation");
  l_pushtime(L, t);
  return 1;
}


static int os_difftime (NameDef(lua_State) *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  NameDef(lua_pushnumber)(L, (NameDef(lua_Number))difftime(t1, t2));
  return 1;
}

/* }====================================================== */


static int os_setlocale (NameDef(lua_State) *L) {
  static const int cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  static const char *const catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", NULL};
  const char *l = luaL_optstring(L, 1, NULL);
  int op = NameDef(luaL_checkoption)(L, 2, "all", catnames);
  NameDef(lua_pushstring)(L, setlocale(cat[op], l));
  return 1;
}


static int os_exit (NameDef(lua_State) *L) {
  int status;
  if (lua_isboolean(L, 1))
    status = (NameDef(lua_toboolean)(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  else
    status = (int)NameDef(luaL_optinteger)(L, 1, EXIT_SUCCESS);
  if (NameDef(lua_toboolean)(L, 2))
    NameDef(lua_close)(L);
  if (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  return 0;
}


static const NameDef(luaL_Reg) syslib[] = {
  {"clock",     os_clock},
  {"date",      os_date},
  {"difftime",  os_difftime},
  {"execute",   os_execute},
  {"exit",      os_exit},
  {"getenv",    os_getenv},
  {"remove",    os_remove},
  {"rename",    os_rename},
  {"setlocale", os_setlocale},
  {"time",      os_time},
  {"tmpname",   os_tmpname},
  {NULL, NULL}
};

/* }====================================================== */



LUAMOD_API int NameDef(luaopen_os) (NameDef(lua_State) *L) {
  luaL_newlib(L, syslib);
  return 1;
}

