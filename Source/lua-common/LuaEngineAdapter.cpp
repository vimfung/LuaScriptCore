//
//  LuaEngineAdapter.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/8/17.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaEngineAdapter.hpp"

#if LUA_VERSION_NUM == 501

#include "lext.h"

#endif

using namespace cn::vimfung::luascriptcore;

lua_State* LuaEngineAdapter::newState()
{
    return luaL_newstate();
}

int LuaEngineAdapter::GC(lua_State *state, int what, int data)
{
    return lua_gc(state, what, data);
}

void LuaEngineAdapter::openLibs (lua_State *state)
{
    luaL_openlibs(state);
}

void LuaEngineAdapter::close (lua_State *state)
{
    lua_close(state);
}

void* LuaEngineAdapter::toUserdata (lua_State *state, int idx)
{
    return lua_touserdata(state, idx);
}

int LuaEngineAdapter::upValueIndex(int index)
{
    return lua_upvalueindex(index);
}

const char* LuaEngineAdapter::toString (lua_State *state, int idx)
{
    return lua_tostring(state, idx);
}

void LuaEngineAdapter::getGlobal (lua_State *state, const char *name)
{
    lua_getglobal(state, name);
}

void LuaEngineAdapter::getField (lua_State *state, int tblIndex, const char *key)
{
    lua_getfield(state, tblIndex, key);
}

void LuaEngineAdapter::pop(lua_State *state, int count)
{
    lua_pop(state, count);
}

void LuaEngineAdapter::pushString (lua_State *state, const char *str)
{
    lua_pushstring(state, str);
}

void LuaEngineAdapter::pushString (lua_State *state, const char *str, size_t len)
{
    lua_pushlstring(state, str, len);
}

void LuaEngineAdapter::setField (lua_State *state, int tblIndex, const char *key)
{
    lua_setfield(state, tblIndex, key);
}

void LuaEngineAdapter::setGlobal (lua_State *state, const char *name)
{
    lua_setglobal(state, name);
}

int LuaEngineAdapter::getTop (lua_State *state)
{
    return lua_gettop(state);
}

int LuaEngineAdapter::loadString (lua_State *state, const char *str)
{
    return luaL_loadstring(state, str);
}

int LuaEngineAdapter::pCall (lua_State *state, int argsCount, int resultsCount, int errfunc)
{
    return lua_pcall(state, argsCount, resultsCount, errfunc);
}

int LuaEngineAdapter::loadFile (lua_State *state, const char *filename)
{
    return luaL_loadfile(state, filename);
}

bool LuaEngineAdapter::isFunction(lua_State *state, int index)
{
    return lua_isfunction(state, index);
}

void LuaEngineAdapter::pushLightUserdata (lua_State *state, void *pointer)
{
    lua_pushlightuserdata(state, pointer);
}

void LuaEngineAdapter::pushCClosure (lua_State *state, lua_CFunction fn, int n)
{
    lua_pushcclosure(state, fn, n);
}

bool LuaEngineAdapter::isNil(lua_State *state, int index)
{
    return lua_isnil(state, index);
}

void LuaEngineAdapter::pushNil (lua_State *state)
{
    lua_pushnil(state);
}

int LuaEngineAdapter::absIndex (lua_State *state, int idx)
{
    return lua_absindex(state, idx);
}

int LuaEngineAdapter::type (lua_State *state, int idx)
{
    return lua_type(state, idx);
}

int LuaEngineAdapter::toBoolean (lua_State *state, int idx)
{
    return lua_toboolean(state, idx);
}

lua_Number LuaEngineAdapter::toNumber(lua_State *state, int idx)
{
    return lua_tonumber(state, idx);
}

const char* LuaEngineAdapter::toLString(lua_State *state, int idx, size_t *len)
{
    return lua_tolstring(state, idx, len);
}

int LuaEngineAdapter::next(lua_State *state, int idx)
{
    return lua_next(state, idx);
}

const void* LuaEngineAdapter::toPointer(lua_State *state, int idx)
{
    return lua_topointer(state, idx);
}

void LuaEngineAdapter::pushValue (lua_State *state, int idx)
{
    lua_pushvalue(state, idx);
}

void LuaEngineAdapter::pushInteger(lua_State *state, lua_Integer n)
{
    lua_pushinteger(state, n);
}

void LuaEngineAdapter::insert (lua_State *state, int idx)
{
    lua_insert(state, idx);
}

bool LuaEngineAdapter::isTable(lua_State *state, int idx)
{
    return lua_istable(state, idx);
}

void LuaEngineAdapter::newTable(lua_State *state)
{
    lua_newtable(state);
}

int LuaEngineAdapter::setMetatable (lua_State *state, int objindex)
{
    return lua_setmetatable(state, objindex);
}

void LuaEngineAdapter::rawSetI (lua_State *state, int idx, int n)
{
    lua_rawseti(state, idx, n);
}

void LuaEngineAdapter::pushNumber(lua_State *state, lua_Number n)
{
    lua_pushnumber(state, n);
}

lua_Integer LuaEngineAdapter::toInteger(lua_State *state, int idx)
{
    return lua_tonumber(state, idx);
}

void LuaEngineAdapter::pushBoolean (lua_State *state, int b)
{
    lua_pushboolean(state, b);
}

int LuaEngineAdapter::isUserdata (lua_State *state, int idx)
{
    return lua_isuserdata(state, idx);
}

void* LuaEngineAdapter::newUserdata (lua_State *state, size_t size)
{
    return lua_newuserdata(state, size);
}

void LuaEngineAdapter::remove (lua_State *state, int idx)
{
    lua_remove(state, idx);
}

int LuaEngineAdapter::getMetatable (lua_State *state, int objindex)
{
    return lua_getmetatable(state, objindex);
}

void LuaEngineAdapter::getMetatable (lua_State *state, const char *tname)
{
    luaL_getmetatable(state, tname);
}

void LuaEngineAdapter::rawSet (lua_State *state, int idx)
{
    lua_rawset(state, idx);
}

void LuaEngineAdapter::rawGet(lua_State *state, int index)
{
    lua_rawget(state, index);
}

const char* LuaEngineAdapter::checkString (lua_State *state, int idx)
{
    return luaL_checkstring(state, idx);
}

int LuaEngineAdapter::newMetatable (lua_State *state, const char *tname)
{
    return luaL_newmetatable(state, tname);
}

void LuaEngineAdapter::pushCFunction (lua_State *state, lua_CFunction fn)
{
    lua_pushcfunction(state, fn);
}

int LuaEngineAdapter::error (lua_State *state, const char *message)
{
    return luaL_error(state, message);
}
