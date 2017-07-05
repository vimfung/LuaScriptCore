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
#include <map>
#include <list>
#include <iostream>
#include <sstream>

using namespace cn::vimfung::luascriptcore;

static int methodRouteHandler(lua_State *state) {

    int returnCount = 0;

    LuaContext *context = (LuaContext *)lua_touserdata(state, lua_upvalueindex(1));
    const char *methodName = lua_tostring(state, lua_upvalueindex(2));

    LuaMethodHandler handler = context-> getMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaSession *session = context -> makeSession(state);

        int top = lua_gettop(state);
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
    lua_State *state = luaL_newstate();
    _dataExchanger = new LuaDataExchanger(this);

    lua_gc(state, LUA_GCSTOP, 0);
    //加载标准库
    luaL_openlibs(state);
    lua_gc(state, LUA_GCRESTART, 0);

    _mainSession = new LuaSession(state, this);
    _currentSession = NULL;
}

LuaContext::~LuaContext()
{
    //释放模块内存
    for (LuaModuleMap::iterator it = _moduleMap.begin(); it != _moduleMap.end() ; ++it)
    {
        LuaModule *module = it -> second;
        module -> release();
    }

    lua_close(_mainSession -> getState());
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
    lua_getglobal(state, "package");
    lua_getfield(state, -1, "path");

    //取出当前路径，并附加新路径
    std::string curPath = lua_tostring(state, -1);
    path = curPath + ";" + path;

    lua_pop(state, 1);
    lua_pushstring(state, path.c_str());
    lua_setfield(state, -2, "path");
    lua_pop(state, 1);
}

void LuaContext::setGlobal(std::string name, LuaValue *value)
{
    lua_State *state = _mainSession -> getState();
    value -> push(this);
    lua_setglobal(state, name.c_str());
}

LuaValue* LuaContext::getGlobal(std::string name)
{
    lua_State *state = _mainSession -> getState();
    lua_getglobal(state, name.c_str());
    return LuaValue::ValueByIndex(this, -1);
}

LuaValue* LuaContext::evalScript(std::string script)
{
    lua_State *state = _mainSession -> getState();
    LuaValue *retValue = NULL;

    int curTop = lua_gettop(state);
    int returnCount = 0;

    luaL_loadstring(state, script.c_str());
    if (lua_pcall(state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(state) - curTop;

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
    lua_pop(state, returnCount);

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return retValue;
}

LuaValue* LuaContext::evalScriptFromFile(std::string path)
{
    lua_State *state = _mainSession -> getState();
    LuaValue *retValue = NULL;

    int curTop = lua_gettop(state);
    int returnCount = 0;

    luaL_loadfile(state, path.c_str());
    if (lua_pcall(state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(state) - curTop;

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
    lua_pop(state, returnCount);

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return retValue;
}

LuaValue* LuaContext::callMethod(std::string methodName, LuaArgumentList *arguments)
{
    lua_State *state = getCurrentSession() -> getState();

    LuaValue *resultValue = NULL;

    int curTop = lua_gettop(state);

    lua_getglobal(state, methodName.c_str());
    if (lua_isfunction(state, -1))
    {
        int returnCount = 0;

        //存在指定方法
        //初始化传递参数
        for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
        {
            LuaValue *item = *i;
            item->push(this);
        }

        if (lua_pcall(state, (int)arguments -> size(), LUA_MULTRET, 0) == 0)
        {
            //调用成功
            returnCount = lua_gettop(state) - curTop;
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

        lua_pop(state, returnCount);
    }
    else
    {
        //将变量从栈中移除
        lua_pop(state, 1);
    }

    if (resultValue == NULL)
    {
        resultValue = LuaValue::NilValue();
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return resultValue;
}

void LuaContext::registerMethod(std::string methodName, LuaMethodHandler handler)
{
    lua_State *state = _mainSession -> getState();
    LuaMethodMap::iterator it =  _methodMap.find(methodName);
    if (it == _methodMap.end())
    {
        _methodMap[methodName] = handler;

        lua_pushlightuserdata(state, this);
        lua_pushstring(state, methodName.c_str());
        lua_pushcclosure(state, methodRouteHandler, 2);
        lua_setglobal(state, methodName.c_str());
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

void LuaContext::registerModule(const std::string &moduleName, LuaModule *module)
{
    if (!this -> isModuleRegisted(moduleName))
    {
        module -> retain();
        module -> onRegister(moduleName, this);
        _moduleMap[moduleName] = module;
    }
}

bool LuaContext::isModuleRegisted(const std::string &moduleName)
{
    lua_State *state = _mainSession -> getState();

    lua_getglobal(state, moduleName.c_str());
    bool retValue = lua_isnil(state, -1);
    lua_pop(state, 1);

    return !retValue;
}

LuaModule* LuaContext::getModule(const std::string &moduleName)
{
    LuaModuleMap::iterator it = _moduleMap.find(moduleName);
    if (it != _moduleMap.end())
    {
        return _moduleMap[moduleName];
    }
    
    return NULL;
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