//
// Created by 冯鸿杰 on 2017/7/5.
//

#include "LuaSession.h"
#include "LuaValue.h"
#include "LuaTuple.h"
#include "LuaEngineAdapter.hpp"
#include "LuaOperationQueue.h"
#include "LuaError.h"
#include "StringUtils.h"
#include "LuaScriptController.h"

#include <sys/time.h>

using namespace cn::vimfung::luascriptcore;

/**
 * 会话表
 */
typedef std::map<std::string, LuaSession*> LuaSessionMap;

/**
 * 钩子会话映射表
 */
static LuaSessionMap _hookSessions;

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
    LuaSessionMap::iterator it = _hookSessions.find(key);
    if (it != _hookSessions.end())
    {
        LuaSession *session = it -> second;
        LuaScriptController *scriptController = session -> getScriptController();

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

LuaSession::LuaSession(lua_State *state, LuaContext *context, bool lightweight)
    : _state(state), _context(context), _lightweight(lightweight), _lastError(NULL), _scriptController(NULL)
{

}

LuaSession::~LuaSession()
{
    clearError();
    setScriptController(NULL);

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

LuaError* LuaSession::getLastError()
{
    return _lastError;
}

LuaScriptController* LuaSession::getScriptController()
{
    return _scriptController;
}

void LuaSession::clearError()
{
    if (_lastError != NULL)
    {
        _lastError -> release();
        _lastError = NULL;
    }
}

void LuaSession::reportLuaException(std::string const& message)
{
    clearError();
    _lastError = new LuaError(this, message);
}

void LuaSession::setScriptController(LuaScriptController *scriptController)
{
    if (scriptController == NULL)
    {
        //取消脚本控制器
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
        LuaSessionMap::iterator it = _hookSessions.find(key);
        if (it != _hookSessions.end())
        {
            _hookSessions.erase(it);
        }

        _context -> getOperationQueue() -> performAction([=](){
            LuaEngineAdapter::setHook(_state, hookLineFunc, 0, 0);
        });

        return;
    }

    std::string key = StringUtils::format("%p", _state);
    LuaSessionMap::iterator it = _hookSessions.find(key);
    if (it != _hookSessions.end())
    {
        LuaSession *oldSession = it -> second;
        oldSession -> setScriptController(NULL);
    }

    if (_scriptController != NULL)
    {
        _scriptController -> release();
    }

    scriptController -> retain();
    _scriptController = scriptController;
    _hookSessions[key] = this;

    _context -> getOperationQueue() -> performAction([=](){
        LuaEngineAdapter::setHook(_state, hookLineFunc, LUA_MASKLINE, 0);
    });

}