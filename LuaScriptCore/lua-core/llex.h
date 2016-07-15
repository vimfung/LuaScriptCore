/*
** $Id: llex.h,v 1.78 2014/10/29 15:38:24 roberto Exp $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ifndef llex_h
#define llex_h

#include "LuaDefine.h"

#include "lobject.h"
#include "lzio.h"


#define FIRST_RESERVED	257


#if !defined(LUA_ENV)
#define LUA_ENV		"_ENV"
#endif


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
enum NameDef(RESERVED) {
  /* terminal symbols denoted by reserved words */
  NameDef(TK_AND) = FIRST_RESERVED, NameDef(TK_BREAK),
  NameDef(TK_DO), NameDef(TK_ELSE), NameDef(TK_ELSEIF), NameDef(TK_END), NameDef(TK_FALSE), NameDef(TK_FOR), NameDef(TK_FUNCTION),
  NameDef(TK_GOTO), NameDef(TK_IF), NameDef(TK_IN), NameDef(TK_LOCAL), NameDef(TK_NIL), NameDef(TK_NOT), NameDef(TK_OR), NameDef(TK_REPEAT),
  NameDef(TK_RETURN), NameDef(TK_THEN), NameDef(TK_TRUE), NameDef(TK_UNTIL), NameDef(TK_WHILE),
  /* other terminal symbols */
  NameDef(TK_IDIV), NameDef(TK_CONCAT), NameDef(TK_DOTS), NameDef(TK_EQ), NameDef(TK_GE), NameDef(TK_LE), NameDef(TK_NE),
  NameDef(TK_SHL), NameDef(TK_SHR),
  NameDef(TK_DBCOLON), NameDef(TK_EOS),
  NameDef(TK_FLT), NameDef(TK_INT), NameDef(TK_NAME), NameDef(TK_STRING)
};

/* number of reserved words */
#define NUM_RESERVED	(cast(int, NameDef(TK_WHILE)-FIRST_RESERVED+1))


typedef union {
  NameDef(lua_Number) r;
  NameDef(lua_Integer) i;
  NameDef(TString) *ts;
} NameDef(SemInfo);  /* semantics information */


typedef struct NameDef(Token) {
  int token;
  NameDef(SemInfo) seminfo;
} Token;


/* state of the lexer plus state of the parser when shared by all
   functions */
typedef struct NameDef(LexState) {
  int current;  /* current character (charint) */
  int linenumber;  /* input line counter */
  int lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  struct NameDef(FuncState) *fs;  /* current function (parser) */
  struct NameDef(lua_State) *L;
  NameDef(ZIO) *z;  /* input stream */
  NameDef(Mbuffer) *buff;  /* buffer for tokens */
  NameDef(Table) *h;  /* to avoid collection/reuse strings */
  struct NameDef(Dyndata) *dyd;  /* dynamic structures used by the parser */
  NameDef(TString) *source;  /* current source name */
  NameDef(TString) *envn;  /* environment variable name */
  char decpoint;  /* locale decimal point */
} NameDef(LexState);


LUAI_FUNC void NameDef(luaX_init) (NameDef(lua_State) *L);
LUAI_FUNC void NameDef(luaX_setinput) (NameDef(lua_State) *L, NameDef(LexState) *ls, NameDef(ZIO) *z,
                              NameDef(TString) *source, int firstchar);
LUAI_FUNC NameDef(TString) *NameDef(luaX_newstring) (NameDef(LexState) *ls, const char *str, size_t l);
LUAI_FUNC void NameDef(luaX_next) (NameDef(LexState) *ls);
LUAI_FUNC int NameDef(luaX_lookahead) (NameDef(LexState) *ls);
LUAI_FUNC l_noret NameDef(luaX_syntaxerror) (NameDef(LexState) *ls, const char *s);
LUAI_FUNC const char *NameDef(luaX_token2str) (NameDef(LexState) *ls, int token);


#endif
