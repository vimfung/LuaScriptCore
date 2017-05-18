//
// Created by 冯鸿杰 on 16/9/23.
//

#include "LuaModule.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaTuple.h"
#include "lua.hpp"
#include <ctype.h>

using namespace cn::vimfung::luascriptcore;

/**
 * 方法路由处理
 */
static int methodRouteHandler(lua_State *state)
{
    int returnCount = 0;

    LuaModule *module = (LuaModule *)lua_touserdata(state, lua_upvalueindex(1));
    const char *methodName = lua_tostring(state, lua_upvalueindex(2));

    LuaModuleMethodHandler handler = module -> getMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaContext *context = module -> getContext();

        int top = lua_gettop(state);
        LuaArgumentList args;
        for (int i = 1; i <= top; i++)
        {
            LuaValue *value = LuaValue::ValueByIndex(context, i);
            args.push_back(value);
        }

        LuaValue *retValue = handler (module, methodName, args);
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

void LuaModule::onRegister (const std::string &name, LuaContext *context)
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

void LuaModule::registerMethod(
        std::string methodName,
        LuaModuleMethodHandler handler)
{
    lua_State *state = getContext() -> getLuaState();
    lua_getglobal(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_pushlightuserdata(state, this);
        lua_pushstring(state, methodName.c_str());
        lua_pushcclosure(state, methodRouteHandler, 2);

        lua_setfield(state, -2, methodName.c_str());

        _methodMap[methodName] = handler;
    }

    lua_pop(state, 1);
}

LuaModuleMethodHandler LuaModule::getMethodHandler(std::string methodName)
{
    LuaModuleMethodMap::iterator it =  _methodMap.find(methodName.c_str());
    if (it != _methodMap.end())
    {
        return it -> second;
    }

    return NULL;
}

const std::string& LuaModule::getName()
{
    return _name;
}

LuaContext* LuaModule::getContext()
{
    return _context;
}
