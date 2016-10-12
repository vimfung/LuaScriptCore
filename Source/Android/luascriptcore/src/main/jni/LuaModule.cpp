//
// Created by 冯鸿杰 on 16/9/23.
//

#include "LuaModule.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "lua.hpp"
#include "LuaDefine.h"
#include "../../../../../lua-core/src/lua.h"
#include <ctype.h>

/**
 * 方法路由处理
 */
static int methodRouteHandler(lua_State *state)
{
    cn::vimfung::luascriptcore::LuaModule *module = (cn::vimfung::luascriptcore::LuaModule *)lua_touserdata(state, lua_upvalueindex(1));
    const char *methodName = lua_tostring(state, lua_upvalueindex(2));

    cn::vimfung::luascriptcore::LuaModuleMethodHandler handler = module -> getMethodHandler(methodName);
    if (handler != NULL)
    {
        cn::vimfung::luascriptcore::LuaContext *context = module -> getContext();

        int top = lua_gettop(state);
        cn::vimfung::luascriptcore::LuaArgumentList args;
        for (int i = 0; i < top; i++)
        {
            cn::vimfung::luascriptcore::LuaValue *value = context -> getValueByIndex(-i - 1);
            args.push_front(value);
        }

        cn::vimfung::luascriptcore::LuaValue *retValue = handler (module, methodName, args);
        if (retValue != NULL)
        {
            retValue -> push(state);
            retValue -> release();
        }

        //释放参数内存
        for (cn::vimfung::luascriptcore::LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            cn::vimfung::luascriptcore::LuaValue *item = *it;
            item -> release();
        }
    }

    return 1;
}

/**
 * 设置器路由处理
 */
static int setterRouteHandler (lua_State *state)
{
    cn::vimfung::luascriptcore::LuaModule *module = (cn::vimfung::luascriptcore::LuaModule *)lua_touserdata(state, lua_upvalueindex(1));
    const char *fieldName = lua_tostring(state, lua_upvalueindex(2));

    cn::vimfung::luascriptcore::LuaModuleSetterHandler handler = module -> getSetterHandler(fieldName);
    if (handler != NULL)
    {
        cn::vimfung::luascriptcore::LuaContext *context = module -> getContext();

        cn::vimfung::luascriptcore::LuaValue *value = NULL;
        int top = lua_gettop(state);
        if (top > 0)
        {
            value = context -> getValueByIndex(1);
        }
        else
        {
            value = new cn::vimfung::luascriptcore::LuaValue();
        }

        handler (module, fieldName, value);

        //释放参数内存
        value -> release();
    }

    return 0;
}

/**
 * 获取器路由处理
 */
static int getterRouteHandler (lua_State *state)
{
    cn::vimfung::luascriptcore::LuaModule *module = (cn::vimfung::luascriptcore::LuaModule *)lua_touserdata(state, lua_upvalueindex(1));
    const char *fieldName = lua_tostring(state, lua_upvalueindex(2));

    cn::vimfung::luascriptcore::LuaModuleGetterHandler handler = module -> getGetterHandler(fieldName);
    if (handler != NULL)
    {
        cn::vimfung::luascriptcore::LuaValue *retValue = handler (module, fieldName);
        if (retValue != NULL)
        {
            retValue -> push(state);
            retValue -> release();
        }
        else
        {
            lua_pushnil(state);
        }
    }

    return 1;
}


void cn::vimfung::luascriptcore::LuaModule::onRegister (const std::string &name, LuaContext *context)
{
    _name = name;
    _context = context;

    //注册模块
    lua_State *state = _context -> getLuaState();
    lua_newtable(state);
    lua_setglobal(state, name.c_str());
}

void cn::vimfung::luascriptcore::LuaModule::registerMethod(
        std::string methodName,
        LuaModuleMethodHandler handler)
{
    lua_State *state = getContext() -> getLuaState();
    lua_getglobal(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, methodName.c_str());
        if (lua_isnil(state, -1))
        {
            //尚未注册
            lua_pop(state, 1);

            lua_pushlightuserdata(state, this);
            lua_pushstring(state, methodName.c_str());
            lua_pushcclosure(state, methodRouteHandler, 2);

            lua_setfield(state, -2, methodName.c_str());

            _methodMap[methodName] = handler;
        }

    }
}

cn::vimfung::luascriptcore::LuaModuleMethodHandler cn::vimfung::luascriptcore::LuaModule::getMethodHandler(std::string methodName)
{
    LuaModuleMethodMap::iterator it =  _methodMap.find(methodName.c_str());
    if (it != _methodMap.end())
    {
        return it -> second;
    }

    return NULL;
}

void cn::vimfung::luascriptcore::LuaModule::registerField(
        std::string fieldName,
        LuaModuleGetterHandler getterHandler,
        LuaModuleSetterHandler setterHandler)
{
    //与iOS中的属性getter和setter方法保持一致, getter直接是属性名称,setter则需要将属性首字母大写并在前面加上set
    char upperCStr[2] = {0};
    upperCStr[0] = toupper(fieldName[0]);
    std::string upperStr = upperCStr;
    std::string fieldNameStr = fieldName.c_str() + 1;
    std::string setterMethodName = "set" + upperStr + fieldNameStr;

    lua_State *state = getContext() -> getLuaState();
    lua_getglobal(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, setterMethodName.c_str());
        if (!lua_isnil(state, -1))
        {
            return;
        }
        lua_pop(state, 1);

        lua_getfield(state, -1, fieldName.c_str());
        if (!lua_isnil(state, -1))
        {
            return;
        }
        lua_pop(state, 1);

        //设置Setter方法
        lua_pushlightuserdata(state, this);
        lua_pushstring(state, fieldName.c_str());
        lua_pushcclosure(state, setterRouteHandler, 2);

        lua_setfield(state, -2, setterMethodName.c_str());

        _setterMap[fieldName] = setterHandler;

        //注册Getter方法
        lua_pushlightuserdata(state, this);
        lua_pushstring(state, fieldName.c_str());
        lua_pushcclosure(state, getterRouteHandler, 2);

        lua_setfield(state, -2, fieldName.c_str());

        _getterMap[fieldName] = getterHandler;
    }
}

cn::vimfung::luascriptcore::LuaModuleSetterHandler cn::vimfung::luascriptcore::LuaModule::getSetterHandler(std::string fieldName)
{
    LuaModuleSetterMap::iterator it =  _setterMap.find(fieldName.c_str());
    if (it != _setterMap.end())
    {
        return it -> second;
    }

    return NULL;
}

cn::vimfung::luascriptcore::LuaModuleGetterHandler cn::vimfung::luascriptcore::LuaModule::getGetterHandler(std::string fieldName)
{
    LuaModuleGetterMap::iterator it =  _getterMap.find(fieldName.c_str());
    if (it != _getterMap.end())
    {
        return it -> second;
    }

    return NULL;
}

const std::string cn::vimfung::luascriptcore::LuaModule::getName()
{
    return (const std::string)_name;
}

cn::vimfung::luascriptcore::LuaContext* cn::vimfung::luascriptcore::LuaModule::getContext()
{
    return _context;
}
