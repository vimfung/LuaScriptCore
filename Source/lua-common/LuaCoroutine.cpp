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
#include "LuaScriptController.h"

using namespace cn::vimfung::luascriptcore;

/**
 * 会话表
 */
typedef std::map<std::string, LuaCoroutine*> LuaCoroutineMap;

/**
 * 钩子会话映射表
 */
static LuaCoroutineMap _hookCoroutines;

/**
 * 获取当前时间戳
 * @return 时间戳
 */
static int64_t getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);    //该函数在sys/time.h头文件中
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void hookLineFunc(lua_State *state, lua_Debug *ar)
{
    std::string key = StringUtils::format("%p", state);
    LuaCoroutineMap::iterator it = _hookCoroutines.find(key);
    if (it != _hookCoroutines.end())
    {
        LuaCoroutine *coroutine = it -> second;
        LuaScriptController *scriptController = coroutine -> getScriptController();

        if (scriptController->isForceExit)
        {
            LuaEngineAdapter::error(state, "script exit...");
        }
        else if (scriptController->timeout > 0)
        {
            if (scriptController->startTime < 1)
            {
                scriptController->startTime = getCurrentTime();
            }

            if (getCurrentTime() - scriptController->startTime > scriptController -> timeout * 1000)
            {
                LuaEngineAdapter::error(state, "script exit...");
            }
        }
    }

}

LuaCoroutine::LuaCoroutine(LuaContext *context)
    : _scriptController(NULL)
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
    setScriptController(NULL);
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

LuaScriptController* LuaCoroutine::getScriptController()
{
    return _scriptController;
}

void LuaCoroutine::setScriptController(LuaScriptController *scriptController)
{
    if (scriptController == NULL)
    {
        if (_scriptController == NULL)
        {
            return;
        }

        if (_scriptController != NULL)
        {
            //重置标识
            _scriptController -> isForceExit = false;
            _scriptController -> startTime = 0;
            _scriptController -> release();
            _scriptController = NULL;
        }

        std::string key = StringUtils::format("%p", _state);
        LuaCoroutineMap::iterator it = _hookCoroutines.find(key);
        if (it != _hookCoroutines.end())
        {
            _hookCoroutines.erase(it);
        }

        LuaEngineAdapter::setHook(_state, hookLineFunc, 0, 0);

        return;
    }

    std::string key = StringUtils::format("%p", _state);
    LuaCoroutineMap::iterator it = _hookCoroutines.find(key);
    if (it != _hookCoroutines.end())
    {
        LuaCoroutine *oldCoroutine = it -> second;
        oldCoroutine -> setScriptController(NULL);
    }

    if (_scriptController != NULL)
    {
        _scriptController -> release();
    }

    scriptController -> retain();
    _scriptController = scriptController;
    _hookCoroutines[key] = this;

    LuaEngineAdapter::setHook(_state, hookLineFunc, LUA_MASKLINE, 0);
}