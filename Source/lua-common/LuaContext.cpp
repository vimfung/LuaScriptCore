//
// Created by vimfung on 16/8/23.
//

#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaModule.h"
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

using namespace cn::vimfung::luascriptcore;

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


LuaContext::LuaContext()
        : LuaObject()
{
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
}

LuaContext::~LuaContext()
{
    //释放模块内存
    for (LuaModuleMap::iterator it = _moduleMap.begin(); it != _moduleMap.end() ; ++it)
    {
        LuaModule *module = it -> second;
        module -> release();
    }

    LuaEngineAdapter::close(_mainSession -> getState());
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
    if (_mainSession -> getState() != state)
    {
        LuaSession *session = new LuaSession(state, this);
        _currentSession = session;

        return session;
    }

    return _mainSession;
}

void LuaContext::destorySession(LuaSession *session)
{
    if (_currentSession == session)
    {
        _currentSession = NULL;
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

void LuaContext::addSearchPath(std::string path)
{
    lua_State *state = _mainSession -> getState();
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
    lua_State *state = _mainSession -> getState();
    value -> push(this);
    LuaEngineAdapter::setGlobal(state, name.c_str());
}

LuaValue* LuaContext::getGlobal(std::string name)
{
    lua_State *state = _mainSession -> getState();
    LuaEngineAdapter::getGlobal(state, name.c_str());
    return LuaValue::ValueByIndex(this, -1);
}

LuaValue* LuaContext::evalScript(std::string script)
{
    lua_State *state = _mainSession -> getState();
    LuaValue *retValue = NULL;

    int curTop = LuaEngineAdapter::getTop(state);
    int returnCount = 0;

    LuaEngineAdapter::loadString(state, script.c_str());
    if (LuaEngineAdapter::pCall(state, 0, LUA_MULTRET, 0) == 0)
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
        returnCount = 1;

        LuaValue *value = LuaValue::ValueByIndex(this, -1);

        std::string errMessage = value -> toString();
        this -> raiseException(errMessage);

        value -> release();
    }

    //弹出返回值
    LuaEngineAdapter::pop(state, returnCount);

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

    return retValue;
}

LuaValue* LuaContext::evalScriptFromFile(std::string path)
{
    lua_State *state = _mainSession -> getState();
    LuaValue *retValue = NULL;

    int curTop = LuaEngineAdapter::getTop(state);
    int returnCount = 0;

    LuaEngineAdapter::loadFile(state, path.c_str());
    if (LuaEngineAdapter::pCall(state, 0, LUA_MULTRET, 0) == 0)
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
        returnCount = 1;

        LuaValue *value = LuaValue::ValueByIndex(this, -1);

        std::string errMessage = value -> toString();
        this -> raiseException(errMessage);

        value -> release();
    }

    //弹出返回值
    LuaEngineAdapter::pop(state, returnCount);

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

    return retValue;
}

LuaValue* LuaContext::callMethod(std::string methodName, LuaArgumentList *arguments)
{
    lua_State *state = getCurrentSession() -> getState();

    LuaValue *resultValue = NULL;

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

        if (LuaEngineAdapter::pCall(state, (int)arguments -> size(), LUA_MULTRET, 0) == 0)
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
            returnCount = 1;

            LuaValue *value = LuaValue::ValueByIndex(this, -1);
            
            std::string errMessage = value -> toString();
            this -> raiseException(errMessage);

            value -> release();
        }

        LuaEngineAdapter::pop(state, returnCount);
    }
    else
    {
        //将变量从栈中移除
        LuaEngineAdapter::pop(state, 1);
    }

    if (resultValue == NULL)
    {
        resultValue = LuaValue::NilValue();
    }

    //回收内存
    LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

    return resultValue;
}

void LuaContext::registerMethod(std::string methodName, LuaMethodHandler handler)
{
    lua_State *state = _mainSession -> getState();
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
