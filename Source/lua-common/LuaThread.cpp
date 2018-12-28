//
// Created by 冯鸿杰 on 2018/12/25.
//

#include "LuaThread.h"
#include "LuaEngineAdapter.hpp"
#include "LuaSession.h"
#include "StringUtils.h"
#include "LuaDataExchanger.h"
#include "LuaTuple.h"
#include "LuaExportsTypeManager.hpp"

using namespace cn::vimfung::luascriptcore;

LuaThread::LuaThread(LuaContext *context, LuaMethodHandler handler)
    : LuaContext(context -> getExportsTypeManager() -> getPlatform(), [this, context] () -> lua_State* {
    return LuaEngineAdapter::newThread(context -> getCurrentSession() -> getState());
})
{
    _context = context;
    _finished = false;

    _exchangeId = StringUtils::format("%p", this);

    int top = LuaEngineAdapter::getTop(_context -> getCurrentSession() -> getState());
    _context -> getDataExchanger() -> setLuaObject(top, _exchangeId);
    _context -> getDataExchanger() -> retainLuaObject(this);
    LuaEngineAdapter::pop(_context -> getCurrentSession() -> getState(), 1);

    registerMethod("_handler", handler);
}

LuaThread::~LuaThread()
{
    _context -> getDataExchanger() -> releaseLuaObject(this);
}

LuaContext* LuaThread::getContext()
{
    return _context;
}

bool LuaThread::hasFinished()
{
    return _finished;
}

void LuaThread::resume(LuaArgumentList argumentList)
{
    getGlobal("_handler");

    for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; it++)
    {
        LuaValue *arg = *it;
        arg -> push(this);
    }

    int status = LuaEngineAdapter::resumeThread(getCurrentSession() -> getState(), NULL, (int)argumentList.size());
    _finished = status != LUA_YIELD;
}

void LuaThread::yield(LuaValue *resultValue)
{
    int resultCount = 1;
    if (resultValue -> getType() == LuaValueTypeTuple)
    {
        resultCount = (int)resultValue -> toTuple() -> count();
    }

    resultValue -> push(this);
    LuaEngineAdapter::yieldThread(getCurrentSession() -> getState(), resultCount);
}
