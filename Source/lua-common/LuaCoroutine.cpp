//
// Created by 冯鸿杰 on 2018/12/25.
//

#include "LuaCoroutine.h"
#include "LuaEngineAdapter.hpp"
#include "LuaSession.h"
#include "StringUtils.h"
#include "LuaDataExchanger.h"
#include "LuaTuple.h"
#include "LuaExportsTypeManager.hpp"
#include "LuaOperationQueue.h"

using namespace cn::vimfung::luascriptcore;

LuaCoroutine::LuaCoroutine(LuaContext *context)
{
    _context = context;
    _exchangeId = StringUtils::format("%p", this);

    _context -> getOperationQueue() -> performAction([this](){

        _state = LuaEngineAdapter::newThread(_context -> getCurrentSession() -> getState());

        int top = LuaEngineAdapter::getTop(_context -> getCurrentSession() -> getState());
        _context -> getDataExchanger() -> setLuaObject(top, _exchangeId);
        _context -> getDataExchanger() -> retainLuaObject(this);
        LuaEngineAdapter::pop(_context -> getCurrentSession() -> getState(), 1);

    });

}

LuaCoroutine::~LuaCoroutine()
{
    _context -> getDataExchanger() -> releaseLuaObject(this);
}

LuaContext* LuaCoroutine::getContext()
{
    return _context;
}

lua_State* LuaCoroutine::getState()
{
    return _state;
}