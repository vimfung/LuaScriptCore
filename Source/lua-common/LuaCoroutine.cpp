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
#include "LuaDataExchanger.h"
#include "LuaFunction.h"

#if !_WINDOWS

#include <sys/time.h>

#endif

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
 * 钩子协程信号量
 */
static std::mutex _hookCoroutinesMutex;

#if _WINDOWS

static INT64 getCurrentTime()
{
	time_t rawtime;
	return (INT64)(time(&rawtime) * 1000);
}

#else

/**
* 获取当前时间戳
* @return 时间戳
*/
static int64_t getCurrentTime()
{

	struct timeval tv;
	gettimeofday(&tv, NULL);    //该函数在sys/time.h头文件中
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;

}

#endif

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

/**
 * 捕获异常处理器名称
 */
static const char * CatchLuaExceptionHandlerName = "__catchExcepitonHandler";

/**
 * 线程处理器
 * @param coroutine 协程对象
 * @param handler 线程处理器
 * @param arguments 参数列表
 * @param scriptController 脚本控制器
 */
static void threadHandler(LuaCoroutine *coroutine, LuaFunction *handler, LuaArgumentList *arguments, LuaScriptController *scriptController)
{
    LuaContext *context = coroutine -> getContext();
    lua_State *state = coroutine -> getState();

    //设置脚本执行配置
    coroutine->setScriptController(scriptController);

    //获取捕获错误方法索引
    int errFuncIndex = 0;
    LuaEngineAdapter::getGlobal(state, CatchLuaExceptionHandlerName);
    if (LuaEngineAdapter::isFunction(state, -1))
    {
        errFuncIndex = LuaEngineAdapter::getTop(state);
    }
    else
    {
        LuaEngineAdapter::pop(state, 1);
    }

    int top = LuaEngineAdapter::getTop(state);
    context -> getDataExchanger() -> getLuaObject(handler, state, NULL);

    if (LuaEngineAdapter::isFunction(state, -1))
    {

        int returnCount = 0;

        for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
        {
            LuaValue *item = *i;
            context -> getDataExchanger() -> pushStack(item, state, NULL);
        }

        if (LuaEngineAdapter::pCall(state, (int)arguments -> size(), LUA_MULTRET, errFuncIndex) == LUA_OK)
        {
            returnCount = LuaEngineAdapter::getTop(state) - top;
        }
        else
        {
            //调用失败
            returnCount = LuaEngineAdapter::getTop(state) - top;
        }

        //弹出返回值
        LuaEngineAdapter::pop(state, returnCount);
    }
    else
    {
        //弹出func
        LuaEngineAdapter::pop(state, 1);
    }

    //移除异常捕获方法
    LuaEngineAdapter::remove(state, errFuncIndex);

    //释放参数
    for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
    {
        LuaValue *item = *i;
        item -> release();
    }
    delete arguments;

    //取消设置脚本执行配置
    coroutine -> setScriptController(NULL);

    //释放内存
    context -> gc();

    //释放对象
    coroutine -> release();
    handler -> release();

    if (scriptController != NULL)
    {
        scriptController -> release();
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
    std::lock_guard<std::mutex> lck(_hookCoroutinesMutex);

    if (scriptController == NULL)
    {
        if (_scriptController == NULL)
        {
            return;
        }

        //重置标识
        _scriptController -> isForceExit = false;
        _scriptController -> startTime = 0;
        _scriptController -> release();
        _scriptController = NULL;

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

void LuaCoroutine::run(LuaFunction *handler, LuaArgumentList arguments, LuaScriptController *scriptController)
{
    if (handler == NULL)
    {
        return;;
    }

    handler -> retain();

    if (scriptController != NULL)
    {
        scriptController -> retain();
    }


    LuaArgumentList *args = new LuaArgumentList();
    for (LuaArgumentList::iterator i = arguments.begin(); i != arguments.end() ; ++i)
    {
        LuaValue *item = *i;
        item -> retain();
        args -> push_back(item);
    }

    std::thread t(&threadHandler, this, handler, args, scriptController);
    t.detach();

}
