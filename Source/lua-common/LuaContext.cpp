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
static std::vector<LuaContext *> _needsGCContextList;


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
        LuaSession *session = context -> makeSession(state);

        LuaArgumentList args;
        session -> parseArguments(args);

        LuaValue *retValue = handler (context, methodName, args);
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
static LuaValue* catchLuaExceptionHandler (LuaContext *context, std::string methodName, LuaArgumentList arguments)
{
    if (arguments.size() > 0)
    {
        context -> raiseException(arguments[0] -> toString());
    }

    return NULL;
}

#if _WINDOWS

void WINAPI contextGCHandler(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
	unityDebug("gc handler...");

	//重置计时器
	timeKillEvent(wTimerID);

	//进行内存回收
	for (std::vector<LuaContext *>::iterator it = _needsGCContextList.begin(); it != _needsGCContextList.end(); it++)
	{
		LuaContext *context = *it;
		context->gcHandler();
		context->release();       // 回收后释放
	}
	//清空
	_needsGCContextList.clear();
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
    for (std::vector<LuaContext *>::iterator it = _needsGCContextList.begin(); it != _needsGCContextList.end(); it++)
    {
        LuaContext *context = *it;
        context -> gcHandler();
        context -> release();       // 回收后释放
    }
    //清空
    _needsGCContextList.clear();
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
    for (std::vector<LuaContext *>::iterator it = _needsGCContextList.begin(); it != _needsGCContextList.end(); it++)
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

LuaContext::LuaContext()
        : LuaObject()
{
    _isActive = true;
    _exceptionHandler = NULL;
    lua_State *state = LuaEngineAdapter::newState();
    _dataExchanger = new LuaDataExchanger(this);

    LuaEngineAdapter::GC(state, LUA_GCSTOP, 0);
    //加载标准库
    LuaEngineAdapter::openLibs(state);
    LuaEngineAdapter::GC(state, LUA_GCRESTART, 0);

    _mainSession = new LuaSession(state, this);
    _currentSession = NULL;
    
    //初始化类型导出管理器
    _exportsTypeManager = new LuaExportsTypeManager(this);

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
    
    LuaEngineAdapter::close(state);
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

LuaSession* LuaContext::makeSession(lua_State *state)
{
    LuaSession *session = new LuaSession(state, this);
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

void LuaContext::onException(LuaExceptionHandler handler)
{
    _exceptionHandler = handler;
}

void LuaContext::raiseException (std::string message)
{
    if (_exceptionHandler != NULL)
    {
        _exceptionHandler (this, message);
    }
}

int LuaContext::catchException()
{
    lua_State *state = getCurrentSession() -> getState();

    LuaEngineAdapter::getGlobal(state, CatchLuaExceptionHandlerName);
    if (LuaEngineAdapter::isFunction(state, -1))
    {
        return LuaEngineAdapter::getTop(state);
    }

    LuaEngineAdapter::pop(state, 1);

    return 0;
}

void LuaContext::addSearchPath(std::string path)
{
    lua_State *state = getCurrentSession() -> getState();
    LuaEngineAdapter::getGlobal(state, "package");
    LuaEngineAdapter::getField(state, -1, "path");

    //取出当前路径，并附加新路径
    std::string curPath = LuaEngineAdapter::toString(state, -1);
    path = curPath + ";" + path;

    LuaEngineAdapter::pop(state, 1);
    LuaEngineAdapter::pushString(state, path.c_str());
    LuaEngineAdapter::setField(state, -2, "path");
    LuaEngineAdapter::pop(state, 1);
}

void LuaContext::setGlobal(std::string name, LuaValue *value)
{
    lua_State *state = getCurrentSession() -> getState();
    value -> push(this);
    LuaEngineAdapter::setGlobal(state, name.c_str());
}

LuaValue* LuaContext::getGlobal(std::string name)
{
    lua_State *state = getCurrentSession() -> getState();
    LuaEngineAdapter::getGlobal(state, name.c_str());
    return LuaValue::ValueByIndex(this, -1);
}

LuaValue* LuaContext::evalScript(std::string script)
{
    lua_State *state = getCurrentSession() -> getState();
    LuaValue *retValue = NULL;

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

    return retValue;
}

LuaValue* LuaContext::evalScriptFromFile(std::string path)
{
    lua_State *state = getCurrentSession() -> getState();
    LuaValue *retValue = NULL;

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

    return retValue;
}

LuaValue* LuaContext::callMethod(std::string methodName, LuaArgumentList *arguments)
{
    lua_State *state = getCurrentSession() -> getState();

    LuaValue *resultValue = NULL;

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

    return resultValue;
}

void LuaContext::registerMethod(std::string methodName, LuaMethodHandler handler)
{
    lua_State *state = getCurrentSession() -> getState();
    LuaMethodMap::iterator it =  _methodMap.find(methodName);
    if (it == _methodMap.end())
    {
        _methodMap[methodName] = handler;

        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushString(state, methodName.c_str());
        LuaEngineAdapter::pushCClosure(state, methodRouteHandler, 2);
        LuaEngineAdapter::setGlobal(state, methodName.c_str());
    }
}

LuaMethodHandler LuaContext::getMethodHandler(std::string methodName)
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
        LuaEngineAdapter::GC(getCurrentSession() -> getState(), LUA_GCCOLLECT, 0);
        _needGC = false;
    }
}
