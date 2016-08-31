//
// Created by vimfung on 16/8/23.
//

#include "LuaContext.h"
#include <map>
#include <list>
#include "LuaDefine.h"


cn::vimfung::luascriptcore::LuaContext::LuaContext()
{
    LOGI("Create LuaContext");

    _exceptionHandler = NULL;
    _state = luaL_newstate();

    //加载标准库
    luaL_openlibs(_state);
}

cn::vimfung::luascriptcore::LuaContext::~LuaContext()
{
    LOGI("Dealloc LuaContext");

    lua_close(_state);
}

void cn::vimfung::luascriptcore::LuaContext::onException(LuaExceptionHandler handler)
{
    _exceptionHandler = handler;
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

                dictValue[key->toString()] = item;

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
        default:
        {
            //默认为nil
            value = LuaValue::NilValue();
            break;
        }
    }

    return value;
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::evalScript(std::string script)
{
    LOGI("start eval script");

    int curTop = lua_gettop(_state);
    int ret = luaL_loadstring(_state, script.c_str()) ||
    lua_pcall(_state, 0, 1, 0);

    bool res = ret == 0;
    if (!res) {

        //错误时触发异常回调
        LuaValue *value = this->getValueByIndex(-1);

        std::string errMessage = value -> toString();

        LOGI("eval script error = %s", errMessage.c_str());

        if (_exceptionHandler != NULL)
        {
            _exceptionHandler (errMessage);
        }

        lua_pop(_state, 1);

        value -> release();

    } else {

        LOGI("eval script success");

        if (lua_gettop(_state) > curTop) {

            //有返回值
            LuaValue *value = this -> getValueByIndex(-1);
            lua_pop(_state, 1);

            return value;
        }
    }

    return LuaValue::NilValue();
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::evalScriptFromFile(
        std::string path)
{
    int curTop = lua_gettop(_state);
    int ret = luaL_loadfile(_state, path.c_str()) ||
    lua_pcall(_state, 0, 1, 0);

    bool res = ret == 0;
    if (!res)
    {
        //错误时触发异常回调
        LuaValue *value = this->getValueByIndex(-1);

        std::string errMessage = value -> toString();

        LOGI("eval from file error = %s", errMessage.c_str());

        if (_exceptionHandler != NULL)
        {
            _exceptionHandler (errMessage);
        }

        lua_pop(_state, 1);

        value -> release();
    }
    else
    {
        LOGI("eval script success");

        if (lua_gettop(_state) > curTop) {

            //有返回值
            LuaValue *value = this -> getValueByIndex(-1);
            lua_pop(_state, 1);

            return value;
        }
    }

    return LuaValue::NilValue();
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::callMethod(
        std::string methodName, LuaArgumentList arguments)
{
    LOGI("call method");

    LuaValue *resultValue = NULL;

    lua_getglobal(_state, methodName.c_str());
    if (lua_isfunction(_state, -1))
    {
        //存在指定方法

        //初始化传递参数
        for (LuaArgumentList::iterator i = arguments.begin(); i != arguments.end() ; ++i)
        {
            LuaValue *item = *i;
            item->push(_state);
        }

        if (lua_pcall(_state, (int)arguments.size(), 1, 0) == 0)
        {
            //调用成功
            resultValue = getValueByIndex(-1);
        }
        else
        {
            //调用失败
            LuaValue *value = getValueByIndex(-1);
            std::string errMessage = value -> toString();

            if (_exceptionHandler != NULL)
            {
                _exceptionHandler (errMessage);
            }

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

    return resultValue;
}