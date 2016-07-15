/*
** $Id: loslib.c,v 1.57 2015/04/10 17:41:04 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#include "LuaDefine.h"

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
** list of valid conversion specifiers for the 'strftime' function
** ===================================================================
*/
#if !defined(LUA_STRFTIMEOPTIONS)	/* { */

#if defined(LUA_USE_C89)
#define LUA_STRFTIMEOPTIONS	{ "aAbBcdHIjmMpSUwWxXyYz%", "" }
#else  /* C99 specification */
#define LUA_STRFTIMEOPTIONS \
	{ "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%", "", \
	  "E", "cCxXyY",  \
	  "O", "deHImMSuUVwWy" }
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
#define l_checktime(L,a)	((time_t)NameDef(luaL_checkinteger)(L,a))

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

static int getboolfield (NameDef(lua_State) *L, const char *key) {
  int res;
  res = (NameDef(lua_getfield)(L, -1, key) == LUA_TNIL) ? -1 : NameDef(lua_toboolean)(L, -1);
  lua_pop(L, 1);
  return res;
}


static int getfield (NameDef(lua_State) *L, const char *key, int d) {
  int res, isnum;
  NameDef(lua_getfield)(L, -1, key);
  res = (int)NameDef(lua_tointegerx)(L, -1, &isnum);
  if (!isnum) {
    if (d < 0)
      return NameDef(luaL_error)(L, "field '%s' missing in date table", key);
    res = d;
  }
  lua_pop(L, 1);
  return res;
}


static const char *checkoption (NameDef(lua_State) *L, const char *conv, char *buff) {
  static const char *const options[] = LUA_STRFTIMEOPTIONS;
  unsigned int i;
  for (i = 0; i < sizeof(options)/sizeof(options[0]); i += 2) {
    if (*conv != '\0' && strchr(options[i], *conv) != NULL) {
      buff[1] = *conv;
      if (*options[i + 1] == '\0') {  /* one-char conversion specifier? */
        buff[2] = '\0';  /* end buffer */
        return conv + 1;
      }
      else if (*(conv + 1) != '\0' &&
               strchr(options[i + 1], *(conv + 1)) != NULL) {
        buff[2] = *(conv + 1);  /* valid two-char conversion specifier */
        buff[3] = '\0';  /* end buffer */
        return conv + 2;
      }
    }
  }
  NameDef(luaL_argerror)(L, 1,
    NameDef(lua_pushfstring)(L, "invalid conversion specifier '%%%s'", conv));
  return conv;  /* to avoid warnings */
}


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
    NameDef(lua_pushnil)(L);
  else if (strcmp(s, "*t") == 0) {
    NameDef(lua_createtable)(L, 0, 9);  /* 9 = number of fields */
    setfield(L, "sec", stm->tm_sec);
    setfield(L, "min", stm->tm_min);
    setfield(L, "hour", stm->tm_hour);
    setfield(L, "day", stm->tm_mday);
    setfield(L, "month", stm->tm_mon+1);
    setfield(L, "year", stm->tm_year+1900);
    setfield(L, "wday", stm->tm_wday+1);
    setfield(L, "yday", stm->tm_yday+1);
    setboolfield(L, "isdst", stm->tm_isdst);
  }
  else {
    char cc[4];
    NameDef(luaL_Buffer) b;
    cc[0] = '%';
    NameDef(luaL_buffinit)(L, &b);
    while (*s) {
      if (*s != '%')  /* no conversion specifier? */
        luaL_addchar(&b, *s++);
      else {
        size_t reslen;
        char buff[200];  /* should be big enough for any conversion result */
        s = checkoption(L, s + 1, cc);
        reslen = strftime(buff, sizeof(buff), cc, stm);
        NameDef(luaL_addlstring)(&b, buff, reslen);
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
    ts.tm_sec = getfield(L, "sec", 0);
    ts.tm_min = getfield(L, "min", 0);
    ts.tm_hour = getfield(L, "hour", 12);
    ts.tm_mday = getfield(L, "day", -1);
    ts.tm_mon = getfield(L, "month", -1) - 1;
    ts.tm_year = getfield(L, "year", -1) - 1900;
    ts.tm_isdst = getboolfield(L, "isdst");
    t = mktime(&ts);
  }
  if (t != (time_t)(l_timet)t)
    NameDef(luaL_error)(L, "time result cannot be represented in this Lua installation");
  else if (t == (time_t)(-1))
    NameDef(lua_pushnil)(L);
  else
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

