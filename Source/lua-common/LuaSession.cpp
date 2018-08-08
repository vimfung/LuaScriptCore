//
// Created by 冯鸿杰 on 2017/7/5.
//

#include "LuaSession.h"
#include "LuaValue.h"
#include "LuaTuple.h"
#include "LuaEngineAdapter.hpp"
#include "LuaOperationQueue.h"

using namespace cn::vimfung::luascriptcore;

LuaSession::LuaSession(lua_State *state, LuaContext *context, bool lightweight)
    : _state(state), _context(context), _lightweight(lightweight), _hasErr(false)
{

}

LuaSession::~LuaSession()
{
    if (!_lightweight)
    {
        _context -> gc();
    }
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
    parseArguments(argumentList, 1);
}

void LuaSession::parseArguments(LuaArgumentList &argumentList, int fromIndex)
{
    _context -> getOperationQueue() -> performAction([=, &argumentList](){

        int top = LuaEngineAdapter::getTop(_state);
        if (top >= fromIndex)
        {
            for (int i = fromIndex; i <= top; i++)
            {
                LuaValue *value = LuaValue::ValueByIndex(_context, i);
                argumentList.push_back(value);
            }
        }

    });

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
        _context -> getOperationQueue() -> performAction([=](){
            LuaEngineAdapter::pushNil(_state);
        });
    }

    return count;
}

void LuaSession::checkException()
{
    if (_hasErr)
    {
        //清空错误信息
        _hasErr = false;

        _context -> getOperationQueue() -> performAction([=](){
            LuaEngineAdapter::error(getState(), _lastErrMsg.c_str());
        });

        _lastErrMsg = "";
    }
}

void LuaSession::reportLuaException(std::string const& message)
{
    _hasErr = true;
    _lastErrMsg = message;
}
