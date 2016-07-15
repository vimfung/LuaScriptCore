/*
** $Id: lopcodes.c,v 1.55 2015/01/05 13:48:33 roberto Exp $
** Opcodes for Lua virtual machine
** See Copyright Notice in lua.h
*/

#define lopcodes_c
#define LUA_CORE

#include "LuaDefine.h"

#include "lprefix.h"


#include <stddef.h>

#include "lopcodes.h"


/* ORDER OP */

LUAI_DDEF const char *const NameDef(luaP_opnames)[NUM_OPCODES+1] = {
  "MOVE",
  "LOADK",
  "LOADKX",
  "LOADBOOL",
  "LOADNIL",
  "GETUPVAL",
  "GETTABUP",
  "GETTABLE",
  "SETTABUP",
  "SETUPVAL",
  "SETTABLE",
  "NEWTABLE",
  "SELF",
  "ADD",
  "SUB",
  "MUL",
  "MOD",
  "POW",
  "DIV",
  "IDIV",
  "BAND",
  "BOR",
  "BXOR",
  "SHL",
  "SHR",
  "UNM",
  "BNOT",
  "NOT",
  "LEN",
  "CONCAT",
  "JMP",
  "EQ",
  "LT",
  "LE",
  "TEST",
  "TESTSET",
  "CALL",
  "TAILCALL",
  "RETURN",
  "FORLOOP",
  "FORPREP",
  "TFORCALL",
  "TFORLOOP",
  "SETLIST",
  "CLOSURE",
  "VARARG",
  "EXTRAARG",
  NULL
};


#define opmode(t,a,b,c,m) (((t)<<7) | ((a)<<6) | ((b)<<4) | ((c)<<2) | (m))

LUAI_DDEF const NameDef(lu_byte) NameDef(luaP_opmodes)[NUM_OPCODES] = {
/*       T  A    B       C     mode		   opcode	*/
  opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iABC))		/* OP_MOVE */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgN), NameDef(iABx))		/* OP_LOADK */
 ,opmode(0, 1, NameDef(OpArgN), NameDef(OpArgN), NameDef(iABx))		/* OP_LOADKX */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgU), NameDef(iABC))		/* OP_LOADBOOL */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgN), NameDef(iABC))		/* OP_LOADNIL */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgN), NameDef(iABC))		/* OP_GETUPVAL */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgK), NameDef(iABC))		/* OP_GETTABUP */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgK), NameDef(iABC))		/* OP_GETTABLE */
 ,opmode(0, 0, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_SETTABUP */
 ,opmode(0, 0, NameDef(OpArgU), NameDef(OpArgN), NameDef(iABC))		/* OP_SETUPVAL */
 ,opmode(0, 0, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_SETTABLE */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgU), NameDef(iABC))		/* OP_NEWTABLE */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgK), NameDef(iABC))		/* OP_SELF */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_ADD */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_SUB */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_MUL */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_MOD */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_POW */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_DIV */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_IDIV */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_BAND */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_BOR */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_BXOR */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_SHL */
 ,opmode(0, 1, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_SHR */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iABC))		/* OP_UNM */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iABC))		/* OP_BNOT */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iABC))		/* OP_NOT */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iABC))		/* OP_LEN */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgR), NameDef(iABC))		/* OP_CONCAT */
 ,opmode(0, 0, NameDef(OpArgR), NameDef(OpArgN), NameDef(iAsBx))		/* OP_JMP */
 ,opmode(1, 0, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_EQ */
 ,opmode(1, 0, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_LT */
 ,opmode(1, 0, NameDef(OpArgK), NameDef(OpArgK), NameDef(iABC))		/* OP_LE */
 ,opmode(1, 0, NameDef(OpArgN), NameDef(OpArgU), NameDef(iABC))		/* OP_TEST */
 ,opmode(1, 1, NameDef(OpArgR), NameDef(OpArgU), NameDef(iABC))		/* OP_TESTSET */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgU), NameDef(iABC))		/* OP_CALL */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgU), NameDef(iABC))		/* OP_TAILCALL */
 ,opmode(0, 0, NameDef(OpArgU), NameDef(OpArgN), NameDef(iABC))		/* OP_RETURN */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iAsBx))      /* OP_FORLOOP */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iAsBx))		/* OP_FORPREP */
 ,opmode(0, 0, NameDef(OpArgN), NameDef(OpArgU), NameDef(iABC))		/* OP_TFORCALL */
 ,opmode(0, 1, NameDef(OpArgR), NameDef(OpArgN), NameDef(iAsBx))		/* OP_TFORLOOP */
 ,opmode(0, 0, NameDef(OpArgU), NameDef(OpArgU), NameDef(iABC))		/* OP_SETLIST */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgN), NameDef(iABx))		/* OP_CLOSURE */
 ,opmode(0, 1, NameDef(OpArgU), NameDef(OpArgN), NameDef(iABC))		/* OP_VARARG */
 ,opmode(0, 0, NameDef(OpArgU), NameDef(OpArgU), NameDef(iAx))		/* OP_EXTRAARG */
};

