//
// Created by vimfung on 16/8/23.
//

#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaModule.h"
#include "LuaPointer.h"
#include "LuaFunction.h"
#include "LuaTuple.h"
#include "lunity.h"
#include "LuaDataExchanger.h"
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
        int top = lua_gettop(state);
        LuaArgumentList args;
        for (int i = 1; i <= top; i++)
        {
            LuaValue *value = LuaValue::ValueByIndex(context, i);
            args.push_back(value);
        }

        LuaValue *retValue = handler (context, methodName, args);
        if (retValue != NULL)
        {
            if (retValue -> getType() == LuaValueTypeTuple)
            {
                returnCount = (int)retValue -> toTuple() -> count();
            }
            else
            {
                returnCount = 1;
            }

            retValue -> push(context);
            retValue -> release();
        }

        //释放参数内存
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return returnCount;
}


LuaContext::LuaContext()
        : LuaObject()
{
    _exceptionHandler = NULL;
    _state = luaL_newstate();
    _dataExchanger = new LuaDataExchanger(this);

    lua_gc(_state, LUA_GCSTOP, 0);
    //加载标准库
    luaL_openlibs(_state);
    lua_gc(_state, LUA_GCRESTART, 0);
}

LuaContext::~LuaContext()
{
    //释放模块内存
    for (LuaModuleMap::iterator it = _moduleMap.begin(); it != _moduleMap.end() ; ++it)
    {
        LuaModule *module = it -> second;
        module -> release();
    }

    lua_close(_state);
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
    lua_getglobal(_state, "package");
    lua_getfield(_state, -1, "path");

    //取出当前路径，并附加新路径
    std::string curPath = lua_tostring(_state, -1);
    path = curPath + ";" + path;

    lua_pop(_state, 1);
    lua_pushstring(_state, path.c_str());
    lua_setfield(_state, -2, "path");
    lua_pop(_state, 1);
}

void LuaContext::setGlobal(std::string name, LuaValue *value)
{
    value -> push(this);
    lua_setglobal(_state, name.c_str());
}

LuaValue* LuaContext::getGlobal(std::string name)
{
    lua_getglobal(_state, name.c_str());
    return LuaValue::ValueByIndex(this, -1);
}

LuaValue* LuaContext::evalScript(std::string script)
{
    LuaValue *retValue = NULL;

    int curTop = lua_gettop(_state);
    int returnCount = 0;

    luaL_loadstring(_state, script.c_str());
    if (lua_pcall(_state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(_state) - curTop;

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
    lua_pop(_state, returnCount);

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    lua_gc(_state, LUA_GCCOLLECT, 0);

    return retValue;
}

LuaValue* LuaContext::evalScriptFromFile(std::string path)
{
    LuaValue *retValue = NULL;

    int curTop = lua_gettop(_state);
    int returnCount = 0;

    luaL_loadfile(_state, path.c_str());
    if (lua_pcall(_state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(_state) - curTop;

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
    lua_pop(_state, returnCount);

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    lua_gc(_state, LUA_GCCOLLECT, 0);

    return retValue;
}

LuaValue* LuaContext::callMethod(std::string methodName, LuaArgumentList *arguments)
{
    LuaValue *resultValue = NULL;

    int curTop = lua_gettop(_state);

    lua_getglobal(_state, methodName.c_str());
    if (lua_isfunction(_state, -1))
    {
        int returnCount = 0;

        //存在指定方法
        //初始化传递参数
        for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
        {
            LuaValue *item = *i;
            item->push(this);
        }

        if (lua_pcall(_state, (int)arguments -> size(), LUA_MULTRET, 0) == 0)
        {
            //调用成功
            returnCount = lua_gettop(_state) - curTop;
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

        lua_pop(_state, returnCount);
    }
    else
    {
        //将变量从栈中移除
        lua_pop(_state, 1);
    }

    if (resultValue == NULL)
    {
        resultValue = LuaValue::NilValue();
    }

    //回收内存
    lua_gc(_state, LUA_GCCOLLECT, 0);

    return resultValue;
}

void LuaContext::registerMethod(std::string methodName,
                                                            LuaMethodHandler handler)
{
    LuaMethodMap::iterator it =  _methodMap.find(methodName);
    if (it == _methodMap.end())
    {
        _methodMap[methodName] = handler;

        lua_pushlightuserdata(_state, this);
        lua_pushstring(_state, methodName.c_str());
        lua_pushcclosure(_state, methodRouteHandler, 2);
        lua_setglobal(_state, methodName.c_str());
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
    lua_getglobal(_state, moduleName.c_str());
    bool retValue = lua_isnil(_state, -1);
    lua_pop(_state, 1);

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

lua_State* LuaContext::getLuaState()
{
    return _state;
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