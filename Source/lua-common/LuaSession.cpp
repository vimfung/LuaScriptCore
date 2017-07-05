//
// Created by 冯鸿杰 on 2017/7/5.
//

#include "LuaSession.h"
#include "LuaValue.h"
#include "LuaTuple.h"

using namespace cn::vimfung::luascriptcore;

LuaSession::LuaSession(lua_State *state, LuaContext *context)
    : _state(state), _context(context)
{

}

LuaSession::~LuaSession()
{
    lua_gc(_state, LUA_GCCOLLECT, 0);
}

lua_State* LuaSession::getState()
{
    return  _state;
}

LuaContext* LuaSession::getContext()
{
    return _context;
}

void LuaSession::parseArguments(LuaArgumentList &argumentList)
{
    int top = lua_gettop(_state);
    if (top >= 1)
    {
        for (int i = 1; i <= top; i++)
        {
            LuaValue *value = LuaValue::ValueByIndex(_context, i);
            argumentList.push_back(value);
        }
    }
}

int LuaSession::setReturnValue(LuaValue *value)
{
    int count = 0;
    if (value != NULL)
    {
        if (value -> getType() == LuaValueTypeTuple)
        {
            count = (int)value -> toTuple() -> count();
        }
        else
        {
            count = 1;
        }

        value -> push(_context);
    }
    else
    {
        lua_pushnil(_state);
    }

    return count;
}