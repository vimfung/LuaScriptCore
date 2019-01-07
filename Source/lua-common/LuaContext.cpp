//
// Created by vimfung on 16/8/23.
//

#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaPointer.h"
#include "LuaFunction.h"
#include "LuaTuple.h"
#include "LuaDataExchanger.h"
#include "LuaSession.h"
#include "LuaEngineAdapter.hpp"
#include "LuaExportsTypeManager.hpp"
#include "LuaOperationQueue.h"
#include "LuaCoroutine.h"
#include <map>
#include <list>
#include <iostream>
#include <sstream>
#include <thread>
#include <signal.h>

#if _WINDOWS

#include <windows.h>
#pragma comment(lib,"Winmm.lib")

#else

#include <sys/time.h>
#include <unistd.h>

#endif

using namespace cn::vimfung::luascriptcore;

/**
 * 捕获异常处理器名称
 */
static const char * CatchLuaExceptionHandlerName = "__catchExcepitonHandler";

/**
 需要回收内存上下文列表
 */
static std::deque<LuaContext *> _needsGCContextList;

/**
 * 线程处理器
 * @param context 上下文对象
 * @param handler 线程处理器
 * @param arguments 参数列表
 */
static void threadHandler(LuaContext *context ,LuaFunction *handler, LuaArgumentList *arguments)
{
    LuaCoroutine *coroutine = new LuaCoroutine(context);
    lua_State *state = coroutine -> getState();

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
            context -> getDataExchanger() -> pushStack(item);
        }

        if (LuaEngineAdapter::pCall(state, (int)arguments -> size(), LUA_MULTRET, errFuncIndex) == 0)
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

    //释放内存
    context -> gc();
}

/**
 * 方法路由处理器
 *
 * @param state lua状态
 *
 * @return 参数返回数量
 */
static int methodRouteHandler(lua_State *state) {

    int returnCount = 0;

    LuaContext *context = (LuaContext *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    const char *methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(2));

    LuaMethodHandler handler = context-> getMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaSession *session = context -> makeSession(state, false);

        LuaArgumentList args;
        session -> parseArguments(args);

        LuaValue *retValue = handler (context, methodName, args);

        //检测异常
        session -> checkException();

        if (retValue != NULL)
        {
            returnCount = session -> setReturnValue(retValue);

            retValue -> push(context);
            retValue -> release();
        }

        //释放参数内存
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }

        context -> destorySession(session);
    }

    return returnCount;
}

/**
 * 捕获Lua异常处理器
 *
 * @param context 上下文对象
 * @param methodName 方法名称
 * @param arguments 参数列表
 */
static LuaValue* catchLuaExceptionHandler (LuaContext *context, std::string const& methodName, LuaArgumentList arguments)
{
    if (arguments.size() > 0)
    {
        context -> outputExceptionMessage(arguments[0] -> toString());
    }

    return NULL;
}

static void executeGC()
{
    //进行内存回收
    for (std::deque<LuaContext *>::iterator it = _needsGCContextList.begin(); it != _needsGCContextList.end(); it++)
    {
        LuaContext *context = *it;
        context->gcHandler();
        context->release();       // 回收后释放
    }
    //清空
    _needsGCContextList.clear();
}

#if _WINDOWS

void WINAPI contextGCHandler(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
	unityDebug("gc handler...");

	//重置计时器
	timeKillEvent(wTimerID);

    //进行内存回收
    std::thread gcThread(executeGC);
    gcThread.detach();
}

#else
/**
 上下文内存回收处理

 @param signo 信号
 */
static void contextGCHandler(int signo)
{
    //重置计时器
    struct itimerval itv;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 0;
    itv.it_interval = itv.it_value;
    setitimer(ITIMER_REAL, &itv, NULL);

    //进行内存回收
    std::thread gcThread(executeGC);
    gcThread.detach();
}

#endif

/**
 上下文开始回收
 
 @param context 上下文对象
 */
static void contextStartGC(LuaContext *context)
{
    //加入列表
    bool hasExists = false;
    for (std::deque<LuaContext *>::iterator it = _needsGCContextList.begin(); it != _needsGCContextList.end(); it++)
    {
        if (*it == context)
        {
            hasExists = true;
            break;
        }
    }

    if (!hasExists)
    {
        context -> retain();
        _needsGCContextList.push_back(context);

#if _WINDOWS

		MMRESULT gcTimerId = timeSetEvent(100, 1, (LPTIMECALLBACK)contextGCHandler, NULL, TIME_ONESHOT);
		if (NULL == gcTimerId)
		{
			unityDebug("gc timer create error!");
		}

#else

        //监听定时器信号
        signal(SIGALRM, contextGCHandler);

        //开始倒计时进行回收
        struct itimerval itv;
        itv.it_value.tv_sec = 0;
        itv.it_value.tv_usec = 100000;
        itv.it_interval = itv.it_value;
        setitimer(ITIMER_REAL, &itv, NULL);

#endif

    }
}

LuaContext::LuaContext(std::string const& platform)
        : LuaContext(platform, [this]() -> lua_State* {
    return LuaEngineAdapter::newState();
})
{
//    _operationQueue = new LuaOperationQueue();
//
//    _isActive = true;
//    _exceptionHandler = NULL;
//    _dataExchanger = new LuaDataExchanger(this);
//
//    _operationQueue -> performAction([this]() {
//
//        lua_State *state = LuaEngineAdapter::newState();
//
//        LuaEngineAdapter::GC(state, LUA_GCSTOP, 0);
//        //加载标准库
//        LuaEngineAdapter::openLibs(state);
//        LuaEngineAdapter::GC(state, LUA_GCRESTART, 0);
//
//        _mainSession = new LuaSession(state, this, false);
//
//    });
//
//    _currentSession = NULL;
//
//    //初始化类型导出管理器
//    _exportsTypeManager = new LuaExportsTypeManager(this, platform);
//
//    //注册错误捕获方法
//    registerMethod(CatchLuaExceptionHandlerName, catchLuaExceptionHandler);
}

LuaContext::LuaContext(std::string const& platform, std::function<lua_State*(void)> const& createStateHandler)
    : LuaObject()
{
    _operationQueue = new LuaOperationQueue();

    _isActive = true;
    _exceptionHandler = NULL;
    _dataExchanger = new LuaDataExchanger(this);

    _operationQueue -> performAction([this, createStateHandler]() {

        lua_State *state = createStateHandler();

        LuaEngineAdapter::GC(state, LUA_GCSTOP, 0);
        //加载标准库
        LuaEngineAdapter::openLibs(state);
        LuaEngineAdapter::GC(state, LUA_GCRESTART, 0);

        _mainSession = new LuaSession(state, this, false);

    });

    _currentSession = NULL;

    //初始化类型导出管理器
    _exportsTypeManager = new LuaExportsTypeManager(this, platform);

    //注册错误捕获方法
    registerMethod(CatchLuaExceptionHandlerName, catchLuaExceptionHandler);
}

LuaContext::~LuaContext()
{
    _isActive = false;      //标记Context已经销毁
    
    lua_State *state = _mainSession -> getState();
    
    _mainSession -> release();
    _exportsTypeManager -> release();
    _dataExchanger -> release();

    _operationQueue -> performAction([state](){
        LuaEngineAdapter::close(state);
    });

    _operationQueue -> release();
}

LuaSession* LuaContext::getMainSession()
{
    return _mainSession;
}

LuaSession* LuaContext::getCurrentSession()
{
    if (_currentSession != NULL)
    {
        return _currentSession;
    }

    return _mainSession;
}

LuaSession* LuaContext::makeSession(lua_State *state, bool lightweight)
{
    LuaSession *session = new LuaSession(state, this, lightweight);
    session -> prevSession = _currentSession;
    _currentSession = session;

    return getCurrentSession();
}

void LuaContext::destorySession(LuaSession *session)
{
    if (_currentSession == session)
    {
        _currentSession = _currentSession -> prevSession;
    }

    if (_mainSession != session)
    {
        session -> release();
    }
}

void LuaContext::onExportsNativeType(LuaExportsNativeTypeHandler handler)
{
    _exportNativeTypeHandler = handler;
}

void LuaContext::exportsNativeType(std::string const& typeName)
{
    if (_exportNativeTypeHandler != NULL)
    {
        _exportNativeTypeHandler(this, typeName);
    }
}

void LuaContext::onException(LuaExceptionHandler handler)
{
    _exceptionHandler = handler;
}

void LuaContext::raiseException (std::string const& message)
{
    getCurrentSession() -> reportLuaException(message);
    throw std::runtime_error(message);
}

void LuaContext::outputExceptionMessage(std::string const& message)
{
    if (_exceptionHandler != NULL)
    {
        _exceptionHandler (this, message);
    }
}

int LuaContext::catchException()
{
    int index = 0;
    _operationQueue -> performAction([this, &index]() {

        lua_State *state = getCurrentSession() -> getState();

        LuaEngineAdapter::getGlobal(state, CatchLuaExceptionHandlerName);
        if (LuaEngineAdapter::isFunction(state, -1))
        {
            index = LuaEngineAdapter::getTop(state);
        }
        else
        {
            LuaEngineAdapter::pop(state, 1);
        }

    });

    return index;
}

void LuaContext::addSearchPath(std::string const& path)
{
    _operationQueue -> performAction([this, path](){

        lua_State *state = getCurrentSession() -> getState();
        LuaEngineAdapter::getGlobal(state, "package");
        LuaEngineAdapter::getField(state, -1, "path");

        //取出当前路径，并附加新路径
        std::string curPath = LuaEngineAdapter::toString(state, -1);
        curPath = curPath + ";" + path;

        LuaEngineAdapter::pop(state, 1);
        LuaEngineAdapter::pushString(state, curPath.c_str());
        LuaEngineAdapter::setField(state, -2, "path");
        LuaEngineAdapter::pop(state, 1);

    });
}

void LuaContext::setGlobal(std::string const& name, LuaValue *value)
{
    _operationQueue -> performAction([this, &name, &value](){

        lua_State *state = getCurrentSession() -> getState();
        value -> push(this);
        LuaEngineAdapter::setGlobal(state, name.c_str());

    });
}

LuaValue* LuaContext::getGlobal(std::string const& name)
{
    LuaValue *value = NULL;
    _operationQueue -> performAction([this, &name, &value](){

        lua_State *state = getCurrentSession() -> getState();
        LuaEngineAdapter::getGlobal(state, name.c_str());
        value = LuaValue::ValueByIndex(this, -1);

    });

    return value;
}

LuaValue* LuaContext::evalScript(std::string const& script)
{
    LuaValue *retValue = NULL;

    _operationQueue -> performAction([this, &script, &retValue](){

        lua_State *state = getCurrentSession() -> getState();

        int errFuncIndex = catchException();
        int curTop = LuaEngineAdapter::getTop(state);
        int returnCount = 0;

        LuaEngineAdapter::loadString(state, script.c_str());
        if (LuaEngineAdapter::pCall(state, 0, LUA_MULTRET, errFuncIndex) == 0)
        {
            //调用成功
            returnCount = LuaEngineAdapter::getTop(state) - curTop;

            if (returnCount > 1)
            {
                LuaTuple *tuple = new LuaTuple();
                for (int i = 1; i <= returnCount; i++)
                {
                    LuaValue *value = LuaValue::ValueByIndex(this, curTop + i);
                    tuple -> addReturnValue(value);
                    value -> release();
                }

                retValue = LuaValue::TupleValue(tuple);

                tuple -> release();
            }
            else if (returnCount == 1)
            {
                retValue = LuaValue::ValueByIndex(this, -1);
            }
        }
        else
        {
            //调用失败
            returnCount = LuaEngineAdapter::getTop(state) - curTop;
        }

        //弹出返回值
        LuaEngineAdapter::pop(state, returnCount);

        //移除异常捕获方法
        LuaEngineAdapter::remove(state, errFuncIndex);

        if (retValue == NULL)
        {
            retValue = LuaValue::NilValue();
        }

        //释放内存
        gc();

    });

    return retValue;
}

LuaValue* LuaContext::evalScriptFromFile(std::string const& path)
{
    LuaValue *retValue = NULL;

    _operationQueue -> performAction([this, &path, &retValue](){

        lua_State *state = getCurrentSession() -> getState();

        int errFuncIndex = catchException();
        int curTop = LuaEngineAdapter::getTop(state);
        int returnCount = 0;

        LuaEngineAdapter::loadFile(state, path.c_str());
        if (LuaEngineAdapter::pCall(state, 0, LUA_MULTRET, errFuncIndex) == 0)
        {
            //调用成功
            returnCount = LuaEngineAdapter::getTop(state) - curTop;

            if (returnCount > 1)
            {
                LuaTuple *tuple = new LuaTuple();
                for (int i = 1; i <= returnCount; i++)
                {
                    LuaValue *value = LuaValue::ValueByIndex(this, curTop + i);
                    tuple -> addReturnValue(value);
                    value -> release();
                }

                retValue = LuaValue::TupleValue(tuple);

                tuple -> release();
            }
            else if (returnCount == 1)
            {
                retValue = LuaValue::ValueByIndex(this, -1);
            }
        }
        else
        {
            //调用失败
            returnCount = LuaEngineAdapter::getTop(state) - curTop;
        }

        //弹出返回值
        LuaEngineAdapter::pop(state, returnCount);

        //移除异常捕获方法
        LuaEngineAdapter::remove(state, errFuncIndex);

        if (retValue == NULL)
        {
            retValue = LuaValue::NilValue();
        }

        //释放内存
        gc();

    });

    return retValue;
}

LuaValue* LuaContext::callMethod(std::string const& methodName, LuaArgumentList *arguments)
{
    LuaValue *resultValue = NULL;
    _operationQueue -> performAction([this, &methodName, &arguments, &resultValue]() {

        lua_State *state = getCurrentSession() -> getState();

        int errFuncIndex = catchException();
        int curTop = LuaEngineAdapter::getTop(state);

        LuaEngineAdapter::getGlobal(state, methodName.c_str());
        if (LuaEngineAdapter::isFunction(state, -1))
        {
            int returnCount = 0;

            //存在指定方法
            //初始化传递参数
            for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
            {
                LuaValue *item = *i;
                item->push(this);
            }

            if (LuaEngineAdapter::pCall(state, (int)arguments -> size(), LUA_MULTRET, errFuncIndex) == 0)
            {
                //调用成功
                returnCount = LuaEngineAdapter::getTop(state) - curTop;
                if (returnCount > 1)
                {
                    LuaTuple *tuple = new LuaTuple();
                    for (int i = 1; i <= returnCount; i++)
                    {
                        LuaValue *value = LuaValue::ValueByIndex(this, curTop + i);
                        tuple -> addReturnValue(value);
                        value -> release();
                    }

                    resultValue = LuaValue::TupleValue(tuple);

                    tuple -> release();
                }
                else if (returnCount == 1)
                {
                    resultValue = LuaValue::ValueByIndex(this, -1);
                }
            }
            else
            {
                //调用失败
                returnCount = LuaEngineAdapter::getTop(state) - curTop;
            }

            LuaEngineAdapter::pop(state, returnCount);
        }
        else
        {
            //将变量从栈中移除
            LuaEngineAdapter::pop(state, 1);
        }

        //移除异常捕获方法
        LuaEngineAdapter::remove(state, errFuncIndex);

        if (resultValue == NULL)
        {
            resultValue = LuaValue::NilValue();
        }

        //回收内存
        gc();

    });

    return resultValue;
}

void LuaContext::registerMethod(std::string const& methodName, LuaMethodHandler handler)
{

    LuaMethodMap::iterator it =  _methodMap.find(methodName);
    if (it == _methodMap.end())
    {
        _methodMap[methodName] = handler;
        _operationQueue -> performAction([this, &methodName, &handler](){

            lua_State *state = getCurrentSession() -> getState();
            LuaEngineAdapter::pushLightUserdata(state, this);
            LuaEngineAdapter::pushString(state, methodName.c_str());
            LuaEngineAdapter::pushCClosure(state, methodRouteHandler, 2);
            LuaEngineAdapter::setGlobal(state, methodName.c_str());

        });
    }
}

void LuaContext::runThread(LuaFunction *handler, LuaArgumentList *arguments)
{
    //赋值参数列表
    LuaArgumentList *args = new LuaArgumentList();
    for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
    {
        LuaValue *item = *i;
        item -> retain();
        args -> push_back(item);
    }

    std::thread t(&threadHandler, this, handler, args);
    t.detach();
}

LuaMethodHandler LuaContext::getMethodHandler(std::string const& methodName)
{
    LuaMethodMap::iterator it =  _methodMap.find(methodName);
    if (it != _methodMap.end())
    {
        return it -> second;
    }

    return NULL;
}

LuaExportsTypeManager* LuaContext::getExportsTypeManager()
{
    return _exportsTypeManager;
}

LuaDataExchanger* LuaContext::getDataExchanger()
{
    return  _dataExchanger;
}

LuaOperationQueue* LuaContext::getOperationQueue()
{
    return _operationQueue;
}

void LuaContext::retainValue(LuaValue *value)
{
    _dataExchanger -> retainLuaObject(value);
}

void LuaContext::releaseValue(LuaValue *value)
{
    _dataExchanger -> releaseLuaObject(value);
}

bool LuaContext::isActive()
{
    return _isActive;
}

void LuaContext::gc()
{
    if (_isActive && !_needGC)
    {
        //进行定时内存回收检测
        _needGC = true;
        contextStartGC(this);
    }
}

void LuaContext::gcHandler()
{
    if (_isActive)
    {
        _operationQueue -> performAction([this](){
            LuaEngineAdapter::GC(getMainSession() -> getState(), LUA_GCCOLLECT, 0);
        });

        _needGC = false;
    }
}
