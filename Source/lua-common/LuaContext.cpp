//
// Created by vimfung on 16/8/23.
//

#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaModule.h"
#include "LuaPointer.h"
#include <map>
#include <list>
#include <iostream>
#include <sstream>

static int methodRouteHandler(lua_State *state) {

    cn::vimfung::luascriptcore::LuaContext *context = (cn::vimfung::luascriptcore::LuaContext *)lua_touserdata(state, lua_upvalueindex(1));
    const char *methodName = lua_tostring(state, lua_upvalueindex(2));

    cn::vimfung::luascriptcore::LuaMethodHandler handler = context-> getMethodHandler(methodName);
    if (handler != NULL)
    {
        int top = lua_gettop(state);
        cn::vimfung::luascriptcore::LuaArgumentList args;
        for (int i = 0; i < top; i++)
        {
            cn::vimfung::luascriptcore::LuaValue *value = context -> getValueByIndex(-i - 1);
            args.push_front(value);
        }

        cn::vimfung::luascriptcore::LuaValue *retValue = handler (context, methodName, args);
        if (retValue != NULL)
        {
            retValue -> push(context);
            retValue -> release();
        }
        else
        {
            lua_pushnil(state);
        }

        //释放参数内存
        for (cn::vimfung::luascriptcore::LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            cn::vimfung::luascriptcore::LuaValue *item = *it;
            item -> release();
        }
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return 1;
}


cn::vimfung::luascriptcore::LuaContext::LuaContext()
        : LuaObject()
{
    _exceptionHandler = NULL;
    _state = luaL_newstate();

    lua_gc(_state, LUA_GCSTOP, 0);
    //加载标准库
    luaL_openlibs(_state);
    lua_gc(_state, LUA_GCRESTART, 0);
}

cn::vimfung::luascriptcore::LuaContext::~LuaContext()
{
    //释放模块内存
    for (LuaModuleMap::iterator it = _moduleMap.begin(); it != _moduleMap.end() ; ++it)
    {
        LuaModule *module = it -> second;
        module -> release();
    }

    lua_close(_state);
}

void cn::vimfung::luascriptcore::LuaContext::onException(LuaExceptionHandler handler)
{
    _exceptionHandler = handler;
}

void cn::vimfung::luascriptcore::LuaContext::raiseException (std::string message)
{
    if (_exceptionHandler != NULL)
    {
        _exceptionHandler (this, message);
    }
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::getValueByIndex(int index)
{
    LuaValue *value = NULL;
    switch (lua_type(_state, index)) {
        case LUA_TNIL:
        {
            value = LuaValue::NilValue();
            break;
        }
        case LUA_TBOOLEAN:
        {
            value = LuaValue::BooleanValue((bool)lua_toboolean(_state, index));
            break;
        }
        case LUA_TNUMBER:
        {
            value = LuaValue::NumberValue(lua_tonumber(_state, index));
            break;
        }
        case LUA_TSTRING:
        {
            size_t len = 0;
            const char *bytes = lua_tolstring(_state, index, &len);

            if (strlen(bytes) != len)
            {
                //为二进制数据流
                value = LuaValue::DataValue(bytes, len);
            }
            else
            {
                //为字符串
                value = LuaValue::StringValue(bytes);
            }
            break;
        }
        case LUA_TTABLE:
        {
            LuaValueMap dictValue;
            LuaValueList arrayValue;
            bool isArray = true;

            lua_pushnil(_state);
            while (lua_next(_state, -2))
            {
                LuaValue *item = getValueByIndex(-1);
                LuaValue *key = getValueByIndex(-2);

                if (isArray)
                {
                    if (key -> getType() != LuaValueTypeNumber)
                    {
                        //非数组对象，释放数组
                        isArray = false;
                    }
                    else if (key -> getType() == LuaValueTypeNumber)
                    {
                        int arrayIndex = (int)key->toNumber();
                        if (arrayIndex <= 0)
                        {
                            //非数组对象，释放数组
                            isArray = false;
                        }
                        else if (arrayIndex - 1 != arrayValue.size())
                        {
                            //非数组对象，释放数组
                            isArray = false;
                        }
                        else
                        {
                            arrayValue.push_back(item);
                        }
                    }
                }

                switch (key -> getType())
                {
                    case LuaValueTypeNumber:
                    {
                        std::ostringstream out;
                        out << key->toNumber();
                        dictValue[out.str()] = item;
                        break;
                    }
                    case LuaValueTypeString:
                        dictValue[key->toString()] = item;
                        break;
                    default:
                        break;
                }

                key->release();

                lua_pop(_state, 1);
            }

            if (isArray)
            {
                value = LuaValue::ArrayValue(arrayValue);
            }
            else
            {
                value = LuaValue::DictonaryValue(dictValue);
            }

            break;
        }
        case LUA_TLIGHTUSERDATA:
        {
            LuaUserdataRef ref = (LuaUserdataRef)lua_topointer(_state, index);
            LuaPointer *pointer = new LuaPointer(ref);
            value = LuaValue::PointerValue(pointer);
            pointer -> release();

            break;
        }
        case LUA_TUSERDATA:
        {
            LuaUserdataRef ref = (LuaUserdataRef)lua_touserdata(_state, index);
            value = LuaValue::ObjectValue((LuaObjectDescriptor *)ref -> value);
            break;
        }
        case LUA_TFUNCTION:
        {
            LuaFunction *function = new LuaFunction(this, index);
            value = LuaValue::FunctionValue(function);
            function -> release();
            break;
        }
        default:
        {
            //默认为nil
            value = LuaValue::NilValue();
            break;
        }
    }

    return value;
}

void cn::vimfung::luascriptcore::LuaContext::addSearchPath(std::string path)
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

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::evalScript(std::string script)
{
    LuaValue *retValue = NULL;

    int curTop = lua_gettop(_state);
    int ret = luaL_loadstring(_state, script.c_str()) ||
    lua_pcall(_state, 0, 1, 0);

    bool res = ret == 0;
    if (!res) {

        //错误时触发异常回调
        LuaValue *value = this->getValueByIndex(-1);

        std::string errMessage = value -> toString();
        this -> raiseException(errMessage);

        lua_pop(_state, 1);

        value -> release();

    } else {

        if (lua_gettop(_state) > curTop) {

            //有返回值
            retValue = this -> getValueByIndex(-1);
            lua_pop(_state, 1);
        }
    }

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    lua_gc(_state, LUA_GCCOLLECT, 0);

    return retValue;
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::evalScriptFromFile(
        std::string path)
{
    LuaValue *retValue = NULL;

    int curTop = lua_gettop(_state);
    int ret = luaL_loadfile(_state, path.c_str()) ||
    lua_pcall(_state, 0, 1, 0);

    bool res = ret == 0;
    if (!res)
    {
        //错误时触发异常回调
        LuaValue *value = this->getValueByIndex(-1);

        std::string errMessage = value -> toString();
        this -> raiseException(errMessage);

        lua_pop(_state, 1);

        value -> release();
    }
    else
    {
        if (lua_gettop(_state) > curTop) {

            //有返回值
            retValue = this -> getValueByIndex(-1);
            lua_pop(_state, 1);
        }
    }

    if (retValue == NULL)
    {
        retValue = LuaValue::NilValue();
    }

    //释放内存
    lua_gc(_state, LUA_GCCOLLECT, 0);

    return retValue;
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::callMethod(
        std::string methodName, LuaArgumentList *arguments)
{
    LuaValue *resultValue = NULL;

    lua_getglobal(_state, methodName.c_str());
    if (lua_isfunction(_state, -1))
    {
        //存在指定方法

        //初始化传递参数
        for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
        {
            LuaValue *item = *i;
            item->push(this);
        }

        if (lua_pcall(_state, (int)arguments -> size(), 1, 0) == 0)
        {
            //调用成功
            resultValue = getValueByIndex(-1);
        }
        else
        {
            //调用失败
            LuaValue *value = getValueByIndex(-1);
            
            std::string errMessage = value -> toString();
            this -> raiseException(errMessage);

            value -> release();

        }

        lua_pop(_state, 1);
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

void cn::vimfung::luascriptcore::LuaContext::registerMethod(std::string methodName,
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

cn::vimfung::luascriptcore::LuaMethodHandler cn::vimfung::luascriptcore::LuaContext::getMethodHandler(std::string methodName)
{
    LuaMethodMap::iterator it =  _methodMap.find(methodName);
    if (it != _methodMap.end())
    {
        return it -> second;
    }

    return NULL;
}

void cn::vimfung::luascriptcore::LuaContext::registerModule(const std::string &moduleName, LuaModule *module)
{
    if (!this -> isModuleRegisted(moduleName))
    {
        module -> retain();
        module -> onRegister(moduleName, this);
        _moduleMap[moduleName] = module;
    }
}

bool cn::vimfung::luascriptcore::LuaContext::isModuleRegisted(const std::string &moduleName)
{
    lua_getglobal(_state, moduleName.c_str());
    bool retValue = lua_isnil(_state, -1);
    lua_pop(_state, 1);

    return !retValue;
}

cn::vimfung::luascriptcore::LuaModule* cn::vimfung::luascriptcore::LuaContext::getModule(const std::string &moduleName)
{
    return _moduleMap[moduleName];
}

lua_State* cn::vimfung::luascriptcore::LuaContext::getLuaState()
{
    return _state;
}
