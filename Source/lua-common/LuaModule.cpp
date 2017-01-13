//
// Created by 冯鸿杰 on 16/9/23.
//

#include "LuaModule.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "lua.hpp"
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
            retValue -> push(context);
            retValue -> release();
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

void cn::vimfung::luascriptcore::LuaModule::onRegister (const std::string &name, LuaContext *context)
{
    _name = name;
    _context = context;

    //注册模块
    lua_State *state = _context -> getLuaState();
    lua_newtable(state);

    //设置模块名称。since ver 1.3
    lua_pushstring(state, name.c_str());
    lua_setfield(state, -2, "name");

    //写入模块标识
    lua_pushstring(state, "module");
    lua_setfield(state, -2, "_nativeType");

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

const std::string cn::vimfung::luascriptcore::LuaModule::getName()
{
    return (const std::string)_name;
}

cn::vimfung::luascriptcore::LuaContext* cn::vimfung::luascriptcore::LuaModule::getContext()
{
    return _context;
}
