/*
** $Id: lstrlib.c,v 1.229 2015/05/20 17:39:23 roberto Exp $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#include "LuaDefine.h"

#include "lprefix.h"


#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary.
*/
#if !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


/* macro to 'unsign' a character */
#define uchar(c)	((unsigned char)(c))


/*
** Some sizes are better limited to fit in 'int', but must also fit in
** 'size_t'. (We assume that 'lua_Integer' cannot be smaller than 'int'.)
*/
#define MAXSIZE  \
	(sizeof(size_t) < sizeof(int) ? (~(size_t)0) : (size_t)(INT_MAX))




static int str_len (NameDef(lua_State) *L) {
  size_t l;
  NameDef(luaL_checklstring)(L, 1, &l);
  NameDef(lua_pushinteger)(L, (NameDef(lua_Integer))l);
  return 1;
}


/* translate a relative string position: negative means back from end */
static NameDef(lua_Integer) posrelat (NameDef(lua_Integer) pos, size_t len) {
  if (pos >= 0) return pos;
  else if (0u - (size_t)pos > len) return 0;
  else return (NameDef(lua_Integer))len + pos + 1;
}


static int str_sub (NameDef(lua_State) *L) {
  size_t l;
  const char *s = NameDef(luaL_checklstring)(L, 1, &l);
  NameDef(lua_Integer) start = posrelat(NameDef(luaL_checkinteger)(L, 2), l);
  NameDef(lua_Integer) end = posrelat(NameDef(luaL_optinteger)(L, 3, -1), l);
  if (start < 1) start = 1;
  if (end > (NameDef(lua_Integer))l) end = l;
  if (start <= end)
    NameDef(lua_pushlstring)(L, s + start - 1, (size_t)(end - start) + 1);
  else lua_pushliteral(L, "");
  return 1;
}


static int str_reverse (NameDef(lua_State) *L) {
  size_t l, i;
  NameDef(luaL_Buffer) b;
  const char *s = NameDef(luaL_checklstring)(L, 1, &l);
  char *p = NameDef(luaL_buffinitsize)(L, &b, l);
  for (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  NameDef(luaL_pushresultsize)(&b, l);
  return 1;
}


static int str_lower (NameDef(lua_State) *L) {
  size_t l;
  size_t i;
  NameDef(luaL_Buffer) b;
  const char *s = NameDef(luaL_checklstring)(L, 1, &l);
  char *p = NameDef(luaL_buffinitsize)(L, &b, l);
  for (i=0; i<l; i++)
    p[i] = tolower(uchar(s[i]));
  NameDef(luaL_pushresultsize)(&b, l);
  return 1;
}


static int str_upper (NameDef(lua_State) *L) {
  size_t l;
  size_t i;
  NameDef(luaL_Buffer) b;
  const char *s = NameDef(luaL_checklstring)(L, 1, &l);
  char *p = NameDef(luaL_buffinitsize)(L, &b, l);
  for (i=0; i<l; i++)
    p[i] = toupper(uchar(s[i]));
  NameDef(luaL_pushresultsize)(&b, l);
  return 1;
}


static int str_rep (NameDef(lua_State) *L) {
  size_t l, lsep;
  const char *s = NameDef(luaL_checklstring)(L, 1, &l);
  NameDef(lua_Integer) n = NameDef(luaL_checkinteger)(L, 2);
  const char *sep = NameDef(luaL_optlstring)(L, 3, "", &lsep);
  if (n <= 0) lua_pushliteral(L, "");
  else if (l + lsep < l || l + lsep > MAXSIZE / n)  /* may overflow? */
    return NameDef(luaL_error)(L, "resulting string too large");
  else {
    size_t totallen = (size_t)n * l + (size_t)(n - 1) * lsep;
    NameDef(luaL_Buffer) b;
    char *p = NameDef(luaL_buffinitsize)(L, &b, totallen);
    while (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, l * sizeof(char)); p += l;
      if (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * sizeof(char));
        p += lsep;
      }
    }
    memcpy(p, s, l * sizeof(char));  /* last copy (not followed by separator) */
    NameDef(luaL_pushresultsize)(&b, totallen);
  }
  return 1;
}


static int str_byte (NameDef(lua_State) *L) {
  size_t l;
  const char *s = NameDef(luaL_checklstring)(L, 1, &l);
  NameDef(lua_Integer) posi = posrelat(NameDef(luaL_optinteger)(L, 2, 1), l);
  NameDef(lua_Integer) pose = posrelat(NameDef(luaL_optinteger)(L, 3, posi), l);
  int n, i;
  if (posi < 1) posi = 1;
  if (pose > (NameDef(lua_Integer))l) pose = l;
  if (posi > pose) return 0;  /* empty interval; return no values */
  if (pose - posi >= INT_MAX)  /* arithmetic overflow? */
    return NameDef(luaL_error)(L, "string slice too long");
  n = (int)(pose -  posi) + 1;
  NameDef(luaL_checkstack)(L, n, "string slice too long");
  for (i=0; i<n; i++)
    NameDef(lua_pushinteger)(L, uchar(s[posi+i-1]));
  return n;
}


static int str_char (NameDef(lua_State) *L) {
  int n = NameDef(lua_gettop)(L);  /* number of arguments */
  int i;
  NameDef(luaL_Buffer) b;
  char *p = NameDef(luaL_buffinitsize)(L, &b, n);
  for (i=1; i<=n; i++) {
    NameDef(lua_Integer) c = NameDef(luaL_checkinteger)(L, i);
    luaL_argcheck(L, uchar(c) == c, i, "value out of range");
    p[i - 1] = uchar(c);
  }
  NameDef(luaL_pushresultsize)(&b, n);
  return 1;
}


static int writer (NameDef(lua_State) *L, const void *b, size_t size, void *B) {
  (void)L;
  NameDef(luaL_addlstring)((NameDef(luaL_Buffer) *) B, (const char *)b, size);
  return 0;
}


static int str_dump (NameDef(lua_State) *L) {
  NameDef(luaL_Buffer) b;
  int strip = NameDef(lua_toboolean)(L, 2);
  NameDef(luaL_checktype)(L, 1, LUA_TFUNCTION);
  NameDef(lua_settop)(L, 1);
  NameDef(luaL_buffinit)(L,&b);
  if (NameDef(lua_dump)(L, writer, &b, strip) != 0)
    return NameDef(luaL_error)(L, "unable to dump given function");
  NameDef(luaL_pushresult)(&b);
  return 1;
}



/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#define CAP_UNFINISHED	(-1)
#define CAP_POSITION	(-2)


typedef struct NameDef(MatchState) {
  int matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  const char *src_init;  /* init of source string */
  const char *src_end;  /* end ('\0') of source string */
  const char *p_end;  /* end ('\0') of pattern */
  NameDef(lua_State) *L;
  int level;  /* total number of captures (finished or unfinished) */
  struct {
    const char *init;
    ptrdiff_t len;
  } capture[LUA_MAXCAPTURES];
} NameDef(MatchState);


/* recursive function */
static const char *match (NameDef(MatchState) *ms, const char *s, const char *p);


/* maximum recursion depth for 'match' */
#if !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


static int check_capture (NameDef(MatchState) *ms, int l) {
  l -= '1';
  if (l < 0 || l >= ms->level || ms->capture[l].len == CAP_UNFINISHED)
    return NameDef(luaL_error)(ms->L, "invalid capture index %%%d", l + 1);
  return l;
}


static int capture_to_close (NameDef(MatchState) *ms) {
  int level = ms->level;
  for (level--; level>=0; level--)
    if (ms->capture[level].len == CAP_UNFINISHED) return level;
  return NameDef(luaL_error)(ms->L, "invalid pattern capture");
}


static const char *classend (NameDef(MatchState) *ms, const char *p) {
  switch (*p++) {
    case L_ESC: {
      if (p == ms->p_end)
        NameDef(luaL_error)(ms->L, "malformed pattern (ends with '%%')");
      return p+1;
    }
    case '[': {
      if (*p == '^') p++;
      do {  /* look for a ']' */
        if (p == ms->p_end)
          NameDef(luaL_error)(ms->L, "malformed pattern (missing ']')");
        if (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } while (*p != ']');
      return p+1;
    }
    default: {
      return p;
    }
  }
}


static int match_class (int c, int cl) {
  int res;
  switch (tolower(cl)) {
    case 'a' : res = isalpha(c); break;
    case 'c' : res = iscntrl(c); break;
    case 'd' : res = isdigit(c); break;
    case 'g' : res = isgraph(c); break;
    case 'l' : res = islower(c); break;
    case 'p' : res = ispunct(c); break;
    case 's' : res = isspace(c); break;
    case 'u' : res = isupper(c); break;
    case 'w' : res = isalnum(c); break;
    case 'x' : res = isxdigit(c); break;
    case 'z' : res = (c == 0); break;  /* deprecated option */
    default: return (cl == c);
  }
  return (islower(cl) ? res : !res);
}


static int matchbracketclass (int c, const char *p, const char *ec) {
  int sig = 1;
  if (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  while (++p < ec) {
    if (*p == L_ESC) {
      p++;
      if (match_class(c, uchar(*p)))
        return sig;
    }
    else if ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      if (uchar(*(p-2)) <= c && c <= uchar(*p))
        return sig;
    }
    else if (uchar(*p) == c) return sig;
  }
  return !sig;
}


static int singlematch (NameDef(MatchState) *ms, const char *s, const char *p,
                        const char *ep) {
  if (s >= ms->src_end)
    return 0;
  else {
    int c = uchar(*s);
    switch (*p) {
      case '.': return 1;  /* matches any char */
      case L_ESC: return match_class(c, uchar(*(p+1)));
      case '[': return matchbracketclass(c, p, ep-1);
      default:  return (uchar(*p) == c);
    }
  }
}


static const char *matchbalance (NameDef(MatchState) *ms, const char *s,
                                   const char *p) {
  if (p >= ms->p_end - 1)
    NameDef(luaL_error)(ms->L, "malformed pattern (missing arguments to '%%b')");
  if (*s != *p) return NULL;
  else {
    int b = *p;
    int e = *(p+1);
    int cont = 1;
    while (++s < ms->src_end) {
      if (*s == e) {
        if (--cont == 0) return s+1;
      }
      else if (*s == b) cont++;
    }
  }
  return NULL;  /* string ends out of balance */
}


static const char *max_expand (NameDef(MatchState) *ms, const char *s,
                                 const char *p, const char *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  while (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  while (i>=0) {
    const char *res = match(ms, (s+i), ep+1);
    if (res) return res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  return NULL;
}


static const char *min_expand (NameDef(MatchState) *ms, const char *s,
                                 const char *p, const char *ep) {
  for (;;) {
    const char *res = match(ms, s, ep+1);
    if (res != NULL)
      return res;
    else if (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    else return NULL;
  }
}


static const char *start_capture (NameDef(MatchState) *ms, const char *s,
                                    const char *p, int what) {
  const char *res;
  int level = ms->level;
  if (level >= LUA_MAXCAPTURES) NameDef(luaL_error)(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  if ((res=match(ms, s, p)) == NULL)  /* match failed? */
    ms->level--;  /* undo capture */
  return res;
}


static const char *end_capture (NameDef(MatchState) *ms, const char *s,
                                  const char *p) {
  int l = capture_to_close(ms);
  const char *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  if ((res = match(ms, s, p)) == NULL)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  return res;
}


static const char *match_capture (NameDef(MatchState) *ms, const char *s, int l) {
  size_t len;
  l = check_capture(ms, l);
  len = ms->capture[l].len;
  if ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    return s+len;
  else return NULL;
}


static const char *match (NameDef(MatchState) *ms, const char *s, const char *p) {
  if (ms->matchdepth-- == 0)
    NameDef(luaL_error)(ms->L, "pattern too complex");
  init: /* using goto's to optimize tail recursion */
  if (p != ms->p_end) {  /* end of pattern? */
    switch (*p) {
      case '(': {  /* start capture */
        if (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        else
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        break;
      }
      case ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        break;
      }
      case '$': {
        if ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          goto dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : NULL;  /* check end of string */
        break;
      }
      case L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        switch (*(p + 1)) {
          case 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            if (s != NULL) {
              p += 4; goto init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            break;
          }
          case 'f': {  /* frontier? */
            const char *ep; char previous;
            p += 2;
            if (*p != '[')
              NameDef(luaL_error)(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            if (!matchbracketclass(uchar(previous), p, ep - 1) &&
               matchbracketclass(uchar(*s), p, ep - 1)) {
              p = ep; goto init;  /* return match(ms, s, ep); */
            }
            s = NULL;  /* match failed */
            break;
          }
          case '0': case '1': case '2': case '3':
          case '4': case '5': case '6': case '7':
          case '8': case '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, uchar(*(p + 1)));
            if (s != NULL) {
              p += 2; goto init;  /* return match(ms, s, p + 2) */
            }
            break;
          }
          default: goto dflt;
        }
        break;
      }
      default: dflt: {  /* pattern class plus optional suffix */
        const char *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        if (!singlematch(ms, s, p, ep)) {
          if (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; goto init;  /* return match(ms, s, ep + 1); */
          }
          else  /* '+' or no suffix */
            s = NULL;  /* fail */
        }
        else {  /* matched once */
          switch (*ep) {  /* handle optional suffix */
            case '?': {  /* optional */
              const char *res;
              if ((res = match(ms, s + 1, ep + 1)) != NULL)
                s = res;
              else {
                p = ep + 1; goto init;  /* else return match(ms, s, ep + 1); */
              }
              break;
            }
            case '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            case '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              break;
            case '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              break;
            default:  /* no suffix */
              s++; p = ep; goto init;  /* return match(ms, s + 1, ep); */
          }
        }
        break;
      }
    }
  }
  ms->matchdepth++;
  return s;
}



static const char *lmemfind (const char *s1, size_t l1,
                               const char *s2, size_t l2) {
  if (l2 == 0) return s1;  /* empty strings are everywhere */
  else if (l2 > l1) return NULL;  /* avoids a negative 'l1' */
  else {
    const char *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    while (l1 > 0 && (init = (const char *)memchr(s1, *s2, l1)) != NULL) {
      init++;   /* 1st char is already checked */
      if (memcmp(init, s2+1, l2) == 0)
        return init-1;
      else {  /* correct 'l1' and 's1' to try again */
        l1 -= init-s1;
        s1 = init;
      }
    }
    return NULL;  /* not found */
  }
}


static void push_onecapture (NameDef(MatchState) *ms, int i, const char *s,
                                                    const char *e) {
  if (i >= ms->level) {
    if (i == 0)  /* ms->level == 0, too */
      NameDef(lua_pushlstring)(ms->L, s, e - s);  /* add whole match */
    else
      NameDef(luaL_error)(ms->L, "invalid capture index %%%d", i + 1);
  }
  else {
    ptrdiff_t l = ms->capture[i].len;
    if (l == CAP_UNFINISHED) NameDef(luaL_error)(ms->L, "unfinished capture");
    if (l == CAP_POSITION)
      NameDef(lua_pushinteger)(ms->L, (ms->capture[i].init - ms->src_init) + 1);
    else
      NameDef(lua_pushlstring)(ms->L, ms->capture[i].init, l);
  }
}


static int push_captures (NameDef(MatchState) *ms, const char *s, const char *e) {
  int i;
  int nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  NameDef(luaL_checkstack)(ms->L, nlevels, "too many captures");
  for (i = 0; i < nlevels; i++)
    push_onecapture(ms, i, s, e);
  return nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
static int nospecials (const char *p, size_t l) {
  size_t upto = 0;
  do {
    if (strpbrk(p + upto, SPECIALS))
      return 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } while (upto <= l);
  return 1;  /* no special chars found */
}


static int str_find_aux (NameDef(lua_State) *L, int find) {
  size_t ls, lp;
  const char *s = NameDef(luaL_checklstring)(L, 1, &ls);
  const char *p = NameDef(luaL_checklstring)(L, 2, &lp);
  NameDef(lua_Integer) init = posrelat(NameDef(luaL_optinteger)(L, 3, 1), ls);
  if (init < 1) init = 1;
  else if (init > (NameDef(lua_Integer))ls + 1) {  /* start after string's end? */
    NameDef(lua_pushnil)(L);  /* cannot find anything */
    return 1;
  }
  /* explicit request or no special characters? */
  if (find && (NameDef(lua_toboolean)(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    const char *s2 = lmemfind(s + init - 1, ls - (size_t)init + 1, p, lp);
    if (s2) {
      NameDef(lua_pushinteger)(L, (s2 - s) + 1);
      NameDef(lua_pushinteger)(L, (s2 - s) + lp);
      return 2;
    }
  }
  else {
    NameDef(MatchState) ms;
    const char *s1 = s + init - 1;
    int anchor = (*p == '^');
    if (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    ms.L = L;
    ms.matchdepth = MAXCCALLS;
    ms.src_init = s;
    ms.src_end = s + ls;
    ms.p_end = p + lp;
    do {
      const char *res;
      ms.level = 0;
      lua_assert(ms.matchdepth == MAXCCALLS);
      if ((res=match(&ms, s1, p)) != NULL) {
        if (find) {
          NameDef(lua_pushinteger)(L, (s1 - s) + 1);  /* start */
          NameDef(lua_pushinteger)(L, res - s);   /* end */
          return push_captures(&ms, NULL, 0) + 2;
        }
        else
          return push_captures(&ms, s1, res);
      }
    } while (s1++ < ms.src_end && !anchor);
  }
  NameDef(lua_pushnil)(L);  /* not found */
  return 1;
}


static int str_find (NameDef(lua_State) *L) {
  return str_find_aux(L, 1);
}


static int str_match (NameDef(lua_State) *L) {
  return str_find_aux(L, 0);
}


static int gmatch_aux (NameDef(lua_State) *L) {
  NameDef(MatchState) ms;
  size_t ls, lp;
  const char *s = NameDef(lua_tolstring)(L, lua_upvalueindex(1), &ls);
  const char *p = NameDef(lua_tolstring)(L, lua_upvalueindex(2), &lp);
  const char *src;
  ms.L = L;
  ms.matchdepth = MAXCCALLS;
  ms.src_init = s;
  ms.src_end = s+ls;
  ms.p_end = p + lp;
  for (src = s + (size_t)lua_tointeger(L, lua_upvalueindex(3));
       src <= ms.src_end;
       src++) {
    const char *e;
    ms.level = 0;
    lua_assert(ms.matchdepth == MAXCCALLS);
    if ((e = match(&ms, src, p)) != NULL) {
      NameDef(lua_Integer) newstart = e-s;
      if (e == src) newstart++;  /* empty match? go at least one position */
      NameDef(lua_pushinteger)(L, newstart);
      lua_replace(L, lua_upvalueindex(3));
      return push_captures(&ms, src, e);
    }
  }
  return 0;  /* not found */
}


static int gmatch (NameDef(lua_State) *L) {
  luaL_checkstring(L, 1);
  luaL_checkstring(L, 2);
  NameDef(lua_settop)(L, 2);
  NameDef(lua_pushinteger)(L, 0);
  NameDef(lua_pushcclosure)(L, gmatch_aux, 3);
  return 1;
}


static void add_s (NameDef(MatchState) *ms, NameDef(luaL_Buffer) *b, const char *s,
                                                   const char *e) {
  size_t l, i;
  NameDef(lua_State) *L = ms->L;
  const char *news = NameDef(lua_tolstring)(L, 3, &l);
  for (i = 0; i < l; i++) {
    if (news[i] != L_ESC)
      luaL_addchar(b, news[i]);
    else {
      i++;  /* skip ESC */
      if (!isdigit(uchar(news[i]))) {
        if (news[i] != L_ESC)
          NameDef(luaL_error)(L, "invalid use of '%c' in replacement string", L_ESC);
        luaL_addchar(b, news[i]);
      }
      else if (news[i] == '0')
          NameDef(luaL_addlstring)(b, s, e - s);
      else {
        push_onecapture(ms, news[i] - '1', s, e);
        NameDef(luaL_tolstring)(L, -1, NULL);  /* if number, convert it to string */
        lua_remove(L, -2);  /* remove original value */
        NameDef(luaL_addvalue)(b);  /* add capture to accumulated result */
      }
    }
  }
}


static void add_value (NameDef(MatchState) *ms, NameDef(luaL_Buffer) *b, const char *s,
                                       const char *e, int tr) {
  NameDef(lua_State) *L = ms->L;
  switch (tr) {
    case LUA_TFUNCTION: {
      int n;
      NameDef(lua_pushvalue)(L, 3);
      n = push_captures(ms, s, e);
      lua_call(L, n, 1);
      break;
    }
    case LUA_TTABLE: {
      push_onecapture(ms, 0, s, e);
      NameDef(lua_gettable)(L, 3);
      break;
    }
    default: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);
      return;
    }
  }
  if (!NameDef(lua_toboolean)(L, -1)) {  /* nil or false? */
    lua_pop(L, 1);
    NameDef(lua_pushlstring)(L, s, e - s);  /* keep original text */
  }
  else if (!NameDef(lua_isstring)(L, -1))
    NameDef(luaL_error)(L, "invalid replacement value (a %s)", luaL_typename(L, -1));
  NameDef(luaL_addvalue)(b);  /* add result to accumulator */
}


static int str_gsub (NameDef(lua_State) *L) {
  size_t srcl, lp;
  const char *src = NameDef(luaL_checklstring)(L, 1, &srcl);
  const char *p = NameDef(luaL_checklstring)(L, 2, &lp);
  int tr = NameDef(lua_type)(L, 3);
  NameDef(lua_Integer) max_s = NameDef(luaL_optinteger)(L, 4, srcl + 1);
  int anchor = (*p == '^');
  NameDef(lua_Integer) n = 0;
  NameDef(MatchState) ms;
  NameDef(luaL_Buffer) b;
  luaL_argcheck(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table expected");
  NameDef(luaL_buffinit)(L, &b);
  if (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  ms.L = L;
  ms.matchdepth = MAXCCALLS;
  ms.src_init = src;
  ms.src_end = src+srcl;
  ms.p_end = p + lp;
  while (n < max_s) {
    const char *e;
    ms.level = 0;
    lua_assert(ms.matchdepth == MAXCCALLS);
    e = match(&ms, src, p);
    if (e) {
      n++;
      add_value(&ms, &b, src, e, tr);
    }
    if (e && e>src) /* non empty match? */
      src = e;  /* skip it */
    else if (src < ms.src_end)
      luaL_addchar(&b, *src++);
    else break;
    if (anchor) break;
  }
  NameDef(luaL_addlstring)(&b, src, ms.src_end-src);
  NameDef(luaL_pushresult)(&b);
  NameDef(lua_pushinteger)(L, n);  /* number of substitutions */
  return 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#if !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#include <locale.h>
#include <math.h>

#define SIZELENMOD	(sizeof(LUA_NUMBER_FRMLEN)/sizeof(char))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#define L_NBFD		((l_mathlim(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
static lua_Number adddigit (char *buff, int n, lua_Number x) {
  lua_Number dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  int d = (int)dd;
  buff[n] = (d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  return x - dd;  /* return what is left */
}


static int num2straux (char *buff, lua_Number x) {
  if (x != x || x == HUGE_VAL || x == -HUGE_VAL)  /* inf or NaN? */
    return sprintf(buff, LUA_NUMBER_FMT, x);  /* equal to '%g' */
  else if (x == 0) {  /* can be -0... */
    sprintf(buff, LUA_NUMBER_FMT, x);
    strcat(buff, "x0p+0");  /* reuses '0/-0' from 'sprintf'... */
    return strlen(buff);
  }
  else {
    int e;
    lua_Number m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    int n = 0;  /* character count */
    if (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add signal */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    if (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      do {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } while (m > 0);
    }
    n += sprintf(buff + n, "p%+d", e);  /* add exponent */
    return n;
  }
}


static int lua_number2strx (lua_State *L, char *buff, const char *fmt,
                            lua_Number x) {
  int n = num2straux(buff, x);
  if (fmt[SIZELENMOD] == 'A') {
    int i;
    for (i = 0; i < n; i++)
      buff[i] = toupper(uchar(buff[i]));
  }
  else if (fmt[SIZELENMOD] != 'a')
    luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  return n;
}

#endif				/* } */


/*
** Maximum size of each formatted item. This maximum size is produced
** by format('%.99f', minfloat), and is equal to 99 + 2 ('-' and '.') +
** number of decimal digits to represent minfloat.
*/
#define MAX_ITEM	(120 + l_mathlim(MAX_10_EXP))


/* valid flags in a format specification */
#define FLAGS	"-+ #0"

/*
** maximum size of each format specification (such as "%-099.99d")
*/
#define MAX_FORMAT	32


static void addquoted (NameDef(lua_State) *L, NameDef(luaL_Buffer) *b, int arg) {
  size_t l;
  const char *s = NameDef(luaL_checklstring)(L, arg, &l);
  luaL_addchar(b, '"');
  while (l--) {
    if (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    else if (*s == '\0' || iscntrl(uchar(*s))) {
      char buff[10];
      if (!isdigit(uchar(*(s+1))))
        sprintf(buff, "\\%d", (int)uchar(*s));
      else
        sprintf(buff, "\\%03d", (int)uchar(*s));
      NameDef(luaL_addstring)(b, buff);
    }
    else
      luaL_addchar(b, *s);
    s++;
  }
  luaL_addchar(b, '"');
}

static const char *scanformat (NameDef(lua_State) *L, const char *strfrmt, char *form) {
  const char *p = strfrmt;
  while (*p != '\0' && strchr(FLAGS, *p) != NULL) p++;  /* skip flags */
  if ((size_t)(p - strfrmt) >= sizeof(FLAGS)/sizeof(char))
    NameDef(luaL_error)(L, "invalid format (repeated flags)");
  if (isdigit(uchar(*p))) p++;  /* skip width */
  if (isdigit(uchar(*p))) p++;  /* (2 digits at most) */
  if (*p == '.') {
    p++;
    if (isdigit(uchar(*p))) p++;  /* skip precision */
    if (isdigit(uchar(*p))) p++;  /* (2 digits at most) */
  }
  if (isdigit(uchar(*p)))
    NameDef(luaL_error)(L, "invalid format (width or precision too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, ((p - strfrmt) + 1) * sizeof(char));
  form += (p - strfrmt) + 1;
  *form = '\0';
  return p;
}


/*
** add length modifier into formats
*/
static void addlenmod (char *form, const char *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  char spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


static int str_format (NameDef(lua_State) *L) {
  int top = NameDef(lua_gettop)(L);
  int arg = 1;
  size_t sfl;
  const char *strfrmt = NameDef(luaL_checklstring)(L, arg, &sfl);
  const char *strfrmt_end = strfrmt+sfl;
  NameDef(luaL_Buffer) b;
  NameDef(luaL_buffinit)(L, &b);
  while (strfrmt < strfrmt_end) {
    if (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    else if (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    else { /* format item */
      char form[MAX_FORMAT];  /* to store the format ('%...') */
      char *buff = NameDef(luaL_prepbuffsize)(&b, MAX_ITEM);  /* to put formatted item */
      int nb = 0;  /* number of bytes in added item */
      if (++arg > top)
        NameDef(luaL_argerror)(L, arg, "no value");
      strfrmt = scanformat(L, strfrmt, form);
      switch (*strfrmt++) {
        case 'c': {
          nb = sprintf(buff, form, (int)NameDef(luaL_checkinteger)(L, arg));
          break;
        }
        case 'd': case 'i':
        case 'o': case 'u': case 'x': case 'X': {
          NameDef(lua_Integer) n = NameDef(luaL_checkinteger)(L, arg);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = sprintf(buff, form, n);
          break;
        }
        case 'a': case 'A':
          addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = lua_number2strx(L, buff, form, NameDef(luaL_checknumber)(L, arg));
          break;
        case 'e': case 'E': case 'f':
        case 'g': case 'G': {
          addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = sprintf(buff, form, NameDef(luaL_checknumber)(L, arg));
          break;
        }
        case 'q': {
          addquoted(L, &b, arg);
          break;
        }
        case 's': {
          size_t l;
          const char *s = NameDef(luaL_tolstring)(L, arg, &l);
          if (!strchr(form, '.') && l >= 100) {
            /* no precision and string is too long to be formatted;
               keep original string */
            NameDef(luaL_addvalue)(&b);
          }
          else {
            nb = sprintf(buff, form, s);
            lua_pop(L, 1);  /* remove result from 'luaL_tolstring' */
          }
          break;
        }
        default: {  /* also treat cases 'pnLlh' */
          return NameDef(luaL_error)(L, "invalid option '%%%c' to 'format'",
                               *(strfrmt - 1));
        }
      }
      luaL_addsize(&b, nb);
    }
  }
  NameDef(luaL_pushresult)(&b);
  return 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#if !defined(LUA_PACKPADBYTE)
#define LUA_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((int)sizeof(NameDef(lua_Integer)))


/* dummy union to get native endianness */
static const union {
  int dummy;
  char little;  /* true iff machine is little endian */
} NameDef(nativeendian) = {1};


/* dummy structure to get native alignment requirements */
struct NameDef(cD) {
  char c;
  union { double d; void *p; NameDef(lua_Integer) i; NameDef(lua_Number) n; } u;
};

#define MAXALIGN	(offsetof(struct NameDef(cD), u))


/*
** Union for serializing floats
*/
typedef union NameDef(Ftypes) {
  float f;
  double d;
  NameDef(lua_Number) n;
  char buff[5 * sizeof(NameDef(lua_Number))];  /* enough for any float type */
} NameDef(Ftypes);


/*
** information to pack/unpack stuff
*/
typedef struct NameDef(Header) {
  NameDef(lua_State) *L;
  int islittle;
  int maxalign;
} NameDef(Header);


/*
** options for pack/unpack
*/
typedef enum NameDef(KOption) {
  NameDef(Kint),		/* signed integers */
  NameDef(Kuint),	/* unsigned integers */
  NameDef(Kfloat),	/* floating-point numbers */
  NameDef(Kchar),	/* fixed-length strings */
  NameDef(Kstring),	/* strings with prefixed length */
  NameDef(Kzstr),	/* zero-terminated strings */
  NameDef(Kpadding),	/* padding */
  NameDef(Kpaddalign),	/* padding for alignment */
  NameDef(Knop)		/* no-op (configuration or spaces) */
} NameDef(KOption);


/*
** Read an integer numeral from string 'fmt' or return 'df' if
** there is no numeral
*/
static int digit (int c) { return '0' <= c && c <= '9'; }

static int getnum (const char **fmt, int df) {
  if (!digit(**fmt))  /* no number? */
    return df;  /* return default value */
  else {
    int a = 0;
    do {
      a = a*10 + (*((*fmt)++) - '0');
    } while (digit(**fmt) && a <= ((int)MAXSIZE - 9)/10);
    return a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size for integers.
*/
static int getnumlimit (NameDef(Header) *h, const char **fmt, int df) {
  int sz = getnum(fmt, df);
  if (sz > MAXINTSIZE || sz <= 0)
    NameDef(luaL_error)(h->L, "integral size (%d) out of limits [1,%d]",
                     sz, MAXINTSIZE);
  return sz;
}


/*
** Initialize Header
*/
static void initheader (NameDef(lua_State) *L, NameDef(Header) *h) {
  h->L = L;
  h->islittle = NameDef(nativeendian).little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
static NameDef(KOption) getoption (NameDef(Header) *h, const char **fmt, int *size) {
  int opt = *((*fmt)++);
  *size = 0;  /* default */
  switch (opt) {
    case 'b': *size = sizeof(char); return NameDef(Kint);
    case 'B': *size = sizeof(char); return NameDef(Kuint);
    case 'h': *size = sizeof(short); return NameDef(Kint);
    case 'H': *size = sizeof(short); return NameDef(Kuint);
    case 'l': *size = sizeof(long); return NameDef(Kint);
    case 'L': *size = sizeof(long); return NameDef(Kuint);
    case 'j': *size = sizeof(NameDef(lua_Integer)); return NameDef(Kint);
    case 'J': *size = sizeof(NameDef(lua_Integer)); return NameDef(Kuint);
    case 'T': *size = sizeof(size_t); return NameDef(Kuint);
    case 'f': *size = sizeof(float); return NameDef(Kfloat);
    case 'd': *size = sizeof(double); return NameDef(Kfloat);
    case 'n': *size = sizeof(NameDef(lua_Number)); return NameDef(Kfloat);
    case 'i': *size = getnumlimit(h, fmt, sizeof(int)); return NameDef(Kint);
    case 'I': *size = getnumlimit(h, fmt, sizeof(int)); return NameDef(Kuint);
    case 's': *size = getnumlimit(h, fmt, sizeof(size_t)); return NameDef(Kstring);
    case 'c':
      *size = getnum(fmt, -1);
      if (*size == -1)
        NameDef(luaL_error)(h->L, "missing size for format option 'c'");
      return NameDef(Kchar);
    case 'z': return NameDef(Kzstr);
    case 'x': *size = 1; return NameDef(Kpadding);
    case 'X': return NameDef(Kpaddalign);
    case ' ': break;
    case '<': h->islittle = 1; break;
    case '>': h->islittle = 0; break;
    case '=': h->islittle = NameDef(nativeendian).little; break;
    case '!': h->maxalign = getnumlimit(h, fmt, MAXALIGN); break;
    default: NameDef(luaL_error)(h->L, "invalid format option '%c'", opt);
  }
  return NameDef(Knop);
}


/*
** Read, classify, and fill other details about the next option.
** 'psize' is filled with option's size, 'notoalign' with its
** alignment requirements.
** Local variable 'size' gets the size to be aligned. (Kpadal option
** always gets its full alignment, other options are limited by 
** the maximum alignment ('maxalign'). Kchar option needs no alignment
** despite its size.
*/
static NameDef(KOption) getdetails (NameDef(Header) *h, size_t totalsize,
                           const char **fmt, int *psize, int *ntoalign) {
  NameDef(KOption) opt = getoption(h, fmt, psize);
  int align = *psize;  /* usually, alignment follows size */
  if (opt == NameDef(Kpaddalign)) {  /* 'X' gets alignment from following option */
    if (**fmt == '\0' || getoption(h, fmt, &align) == NameDef(Kchar) || align == 0)
      NameDef(luaL_argerror)(h->L, 1, "invalid next option for option 'X'");
  }
  if (align <= 1 || opt == NameDef(Kchar))  /* need no alignment? */
    *ntoalign = 0;
  else {
    if (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    if ((align & (align - 1)) != 0)  /* is 'align' not a power of 2? */
      NameDef(luaL_argerror)(h->L, 1, "format asks for alignment not power of 2");
    *ntoalign = (align - (int)(totalsize & (align - 1))) & (align - 1);
  }
  return opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
static void packint (NameDef(luaL_Buffer) *b, NameDef(lua_Unsigned) n,
                     int islittle, int size, int neg) {
  char *buff = NameDef(luaL_prepbuffsize)(b, size);
  int i;
  buff[islittle ? 0 : size - 1] = (char)(n & MC);  /* first byte */
  for (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (char)(n & MC);
  }
  if (neg && size > SZINT) {  /* negative number need sign extension? */
    for (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (char)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
static void copywithendian (volatile char *dest, volatile const char *src,
                            int size, int islittle) {
  if (islittle == NameDef(nativeendian).little) {
    while (size-- != 0)
      *(dest++) = *(src++);
  }
  else {
    dest += size - 1;
    while (size-- != 0)
      *(dest--) = *(src++);
  }
}


static int str_pack (NameDef(lua_State) *L) {
  NameDef(luaL_Buffer) b;
  NameDef(Header) h;
  const char *fmt = luaL_checkstring(L, 1);  /* format string */
  int arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  NameDef(lua_pushnil)(L);  /* mark to separate arguments from string buffer */
  NameDef(luaL_buffinit)(L, &b);
  while (*fmt != '\0') {
    int size, ntoalign;
    NameDef(KOption) opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    totalsize += ntoalign + size;
    while (ntoalign-- > 0)
     luaL_addchar(&b, LUA_PACKPADBYTE);  /* fill alignment */
    arg++;
    switch (opt) {
      case NameDef(Kint): {  /* signed integers */
        NameDef(lua_Integer) n = NameDef(luaL_checkinteger)(L, arg);
        if (size < SZINT) {  /* need overflow check? */
          NameDef(lua_Integer) lim = (NameDef(lua_Integer))1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (NameDef(lua_Unsigned))n, h.islittle, size, (n < 0));
        break;
      }
      case NameDef(Kuint): {  /* unsigned integers */
        NameDef(lua_Integer) n = NameDef(luaL_checkinteger)(L, arg);
        if (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (NameDef(lua_Unsigned))n < ((NameDef(lua_Unsigned))1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (NameDef(lua_Unsigned))n, h.islittle, size, 0);
        break;
      }
      case NameDef(Kfloat): {  /* floating-point options */
        volatile NameDef(Ftypes) u;
        char *buff = NameDef(luaL_prepbuffsize)(&b, size);
        NameDef(lua_Number) n = NameDef(luaL_checknumber)(L, arg);  /* get argument */
        if (size == sizeof(u.f)) u.f = (float)n;  /* copy it into 'u' */
        else if (size == sizeof(u.d)) u.d = (double)n;
        else u.n = n;
        /* move 'u' to final result, correcting endianness if needed */
        copywithendian(buff, u.buff, size, h.islittle);
        luaL_addsize(&b, size);
        break;
      }
      case NameDef(Kchar): {  /* fixed-size string */
        size_t len;
        const char *s = NameDef(luaL_checklstring)(L, arg, &len);
        luaL_argcheck(L, len == (size_t)size, arg, "wrong length");
        NameDef(luaL_addlstring)(&b, s, size);
        break;
      }
      case NameDef(Kstring): {  /* strings with length count */
        size_t len;
        const char *s = NameDef(luaL_checklstring)(L, arg, &len);
        luaL_argcheck(L, size >= (int)sizeof(size_t) ||
                         len < ((size_t)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        packint(&b, (NameDef(lua_Unsigned))len, h.islittle, size, 0);  /* pack length */
        NameDef(luaL_addlstring)(&b, s, len);
        totalsize += len;
        break;
      }
      case NameDef(Kzstr): {  /* zero-terminated string */
        size_t len;
        const char *s = NameDef(luaL_checklstring)(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        NameDef(luaL_addlstring)(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        break;
      }
      case NameDef(Kpadding): luaL_addchar(&b, LUA_PACKPADBYTE);  /* FALLTHROUGH */
      case NameDef(Kpaddalign): case NameDef(Knop):
        arg--;  /* undo increment */
        break;
    }
  }
  NameDef(luaL_pushresult)(&b);
  return 1;
}


static int str_packsize (NameDef(lua_State) *L) {
  NameDef(Header) h;
  const char *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  while (*fmt != '\0') {
    int size, ntoalign;
    NameDef(KOption) opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    size += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAXSIZE - size, 1,
                     "format result too large");
    totalsize += size;
    switch (opt) {
      case NameDef(Kstring):  /* strings with length count */
      case NameDef(Kzstr):    /* zero-terminated string */
        NameDef(luaL_argerror)(L, 1, "variable-length format");
        break;
      default:  break;
    }
  }
  NameDef(lua_pushinteger)(L, (NameDef(lua_Integer))totalsize);
  return 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
static NameDef(lua_Integer) unpackint (NameDef(lua_State) *L, const char *str,
                              int islittle, int size, int issigned) {
  NameDef(lua_Unsigned) res = 0;
  int i;
  int limit = (size  <= SZINT) ? size : SZINT;
  for (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (NameDef(lua_Unsigned))(unsigned char)str[islittle ? i : size - 1 - i];
  }
  if (size < SZINT) {  /* real size smaller than lua_Integer? */
    if (issigned) {  /* needs sign extension? */
      NameDef(lua_Unsigned) mask = (NameDef(lua_Unsigned))1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  else if (size > SZINT) {  /* must check unread bytes */
    int mask = (!issigned || (NameDef(lua_Integer))res >= 0) ? 0 : MC;
    for (i = limit; i < size; i++) {
      if ((unsigned char)str[islittle ? i : size - 1 - i] != mask)
        NameDef(luaL_error)(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  return (NameDef(lua_Integer))res;
}


static int str_unpack (NameDef(lua_State) *L) {
  NameDef(Header) h;
  const char *fmt = luaL_checkstring(L, 1);
  size_t ld;
  const char *data = NameDef(luaL_checklstring)(L, 2, &ld);
  size_t pos = (size_t)posrelat(NameDef(luaL_optinteger)(L, 3, 1), ld) - 1;
  int n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  while (*fmt != '\0') {
    int size, ntoalign;
    NameDef(KOption) opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    if ((size_t)ntoalign + size > ~pos || pos + ntoalign + size > ld)
      NameDef(luaL_argerror)(L, 2, "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    NameDef(luaL_checkstack)(L, 2, "too many results");
    n++;
    switch (opt) {
      case NameDef(Kint):
      case NameDef(Kuint): {
        NameDef(lua_Integer) res = unpackint(L, data + pos, h.islittle, size,
                                       (opt == NameDef(Kint)));
        NameDef(lua_pushinteger)(L, res);
        break;
      }
      case NameDef(Kfloat): {
        volatile NameDef(Ftypes) u;
        NameDef(lua_Number) num;
        copywithendian(u.buff, data + pos, size, h.islittle);
        if (size == sizeof(u.f)) num = (NameDef(lua_Number))u.f;
        else if (size == sizeof(u.d)) num = (NameDef(lua_Number))u.d;
        else num = u.n;
        NameDef(lua_pushnumber)(L, num);
        break;
      }
      case NameDef(Kchar): {
        NameDef(lua_pushlstring)(L, data + pos, size);
        break;
      }
      case NameDef(Kstring): {
        size_t len = (size_t)unpackint(L, data + pos, h.islittle, size, 0);
        luaL_argcheck(L, pos + len + size <= ld, 2, "data string too short");
        NameDef(lua_pushlstring)(L, data + pos + size, len);
        pos += len;  /* skip string */
        break;
      }
      case NameDef(Kzstr): {
        size_t len = (int)strlen(data + pos);
        NameDef(lua_pushlstring)(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        break;
      }
      case NameDef(Kpaddalign): case NameDef(Kpadding): case NameDef(Knop):
        n--;  /* undo increment */
        break;
    }
    pos += size;
  }
  NameDef(lua_pushinteger)(L, pos + 1);  /* next position */
  return n + 1;
}

/* }====================================================== */


static const NameDef(luaL_Reg) strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_find},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_lower},
  {"match", str_match},
  {"rep", str_rep},
  {"reverse", str_reverse},
  {"sub", str_sub},
  {"upper", str_upper},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {NULL, NULL}
};


static void createmetatable (NameDef(lua_State) *L) {
  NameDef(lua_createtable)(L, 0, 1);  /* table to be metatable for strings */
  lua_pushliteral(L, "");  /* dummy string */
  NameDef(lua_pushvalue)(L, -2);  /* copy table */
  NameDef(lua_setmetatable)(L, -2);  /* set table as metatable for strings */
  lua_pop(L, 1);  /* pop dummy string */
  NameDef(lua_pushvalue)(L, -2);  /* get string library */
  NameDef(lua_setfield)(L, -2, "__index");  /* metatable.__index = string */
  lua_pop(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API int NameDef(luaopen_string) (NameDef(lua_State) *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  return 1;
}

