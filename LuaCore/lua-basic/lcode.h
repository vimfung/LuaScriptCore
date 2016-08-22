/*
** $Id: lcode.h,v 1.64 2016/01/05 16:22:37 roberto Exp $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#include "llex.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
typedef enum NameDef(BinOpr) {
  NameDef(OPR_ADD), NameDef(OPR_SUB), NameDef(OPR_MUL), NameDef(OPR_MOD), NameDef(OPR_POW),
  NameDef(OPR_DIV),
  NameDef(OPR_IDIV),
  NameDef(OPR_BAND), NameDef(OPR_BOR), NameDef(OPR_BXOR),
  NameDef(OPR_SHL), NameDef(OPR_SHR),
  NameDef(OPR_CONCAT),
  NameDef(OPR_EQ), NameDef(OPR_LT), NameDef(OPR_LE),
  NameDef(OPR_NE), NameDef(OPR_GT), NameDef(OPR_GE),
  NameDef(OPR_AND), NameDef(OPR_OR),
  NameDef(OPR_NOBINOPR)
} NameDef(BinOpr);

typedef enum NameDef(UnOpr) { NameDef(OPR_MINUS), NameDef(OPR_BNOT), NameDef(OPR_NOT), NameDef(OPR_LEN), NameDef(OPR_NOUNOPR) } NameDef(UnOpr);


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])

#define luaK_codeAsBx(fs,o,A,sBx)	NameDef(luaK_codeABx)(fs,o,A,(sBx)+MAXARG_sBx)

#define luaK_setmultret(fs,e)	NameDef(luaK_setreturns)(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	NameDef(luaK_patchlist)(fs, NameDef(luaK_jump)(fs), t)

LUAI_FUNC int NameDef(luaK_codeABx) (NameDef(FuncState) *fs, NameDef(OpCode) o, int A, unsigned int Bx);
LUAI_FUNC int NameDef(luaK_codeABC) (NameDef(FuncState) *fs, NameDef(OpCode) o, int A, int B, int C);
LUAI_FUNC int NameDef(luaK_codek) (NameDef(FuncState) *fs, int reg, int k);
LUAI_FUNC void NameDef(luaK_fixline) (NameDef(FuncState) *fs, int line);
LUAI_FUNC void NameDef(luaK_nil) (NameDef(FuncState) *fs, int from, int n);
LUAI_FUNC void NameDef(luaK_reserveregs) (NameDef(FuncState) *fs, int n);
LUAI_FUNC void NameDef(luaK_checkstack) (NameDef(FuncState) *fs, int n);
LUAI_FUNC int NameDef(luaK_stringK) (NameDef(FuncState) *fs, NameDef(TString) *s);
LUAI_FUNC int NameDef(luaK_intK) (NameDef(FuncState) *fs, NameDef(lua_Integer) n);
LUAI_FUNC void NameDef(luaK_dischargevars) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC int NameDef(luaK_exp2anyreg) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_exp2anyregup) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_exp2nextreg) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_exp2val) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC int NameDef(luaK_exp2RK) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_self) (NameDef(FuncState) *fs, NameDef(expdesc) *e, NameDef(expdesc) *key);
LUAI_FUNC void NameDef(luaK_indexed) (NameDef(FuncState) *fs, NameDef(expdesc) *t, NameDef(expdesc) *k);
LUAI_FUNC void NameDef(luaK_goiftrue) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_goiffalse) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_storevar) (NameDef(FuncState) *fs, NameDef(expdesc) *var, NameDef(expdesc) *e);
LUAI_FUNC void NameDef(luaK_setreturns) (NameDef(FuncState) *fs, NameDef(expdesc) *e, int nresults);
LUAI_FUNC void NameDef(luaK_setoneret) (NameDef(FuncState) *fs, NameDef(expdesc) *e);
LUAI_FUNC int NameDef(luaK_jump) (NameDef(FuncState) *fs);
LUAI_FUNC void NameDef(luaK_ret) (NameDef(FuncState) *fs, int first, int nret);
LUAI_FUNC void NameDef(luaK_patchlist) (NameDef(FuncState) *fs, int list, int target);
LUAI_FUNC void NameDef(luaK_patchtohere) (NameDef(FuncState) *fs, int list);
LUAI_FUNC void NameDef(luaK_patchclose) (NameDef(FuncState) *fs, int list, int level);
LUAI_FUNC void NameDef(luaK_concat) (NameDef(FuncState) *fs, int *l1, int l2);
LUAI_FUNC int NameDef(luaK_getlabel) (NameDef(FuncState) *fs);
LUAI_FUNC void NameDef(luaK_prefix) (NameDef(FuncState) *fs, NameDef(UnOpr) op, NameDef(expdesc) *v, int line);
LUAI_FUNC void NameDef(luaK_infix) (NameDef(FuncState) *fs, NameDef(BinOpr) op, NameDef(expdesc) *v);
LUAI_FUNC void NameDef(luaK_posfix) (NameDef(FuncState) *fs, NameDef(BinOpr) op, NameDef(expdesc) *v1,
                            NameDef(expdesc) *v2, int line);
LUAI_FUNC void NameDef(luaK_setlist) (NameDef(FuncState) *fs, int base, int nelems, int tostore);


#endif
