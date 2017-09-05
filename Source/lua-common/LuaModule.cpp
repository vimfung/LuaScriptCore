//
// Created by 冯鸿杰 on 16/9/23.
//

#include "LuaModule.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaTuple.h"
#include "LuaSession.h"
#include "lua.hpp"
#include "LuaEngineAdapter.hpp"
#include <ctype.h>

using namespace cn::vimfung::luascriptcore;

/**
 * 方法路由处理
 */
static int methodRouteHandler(lua_State *state)
{
    int returnCount = 0;

    LuaModule *module = (LuaModule *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    const char *methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(2));

    LuaModuleMethodHandler handler = module -> getMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaContext *context = module -> getContext();
        LuaSession *session = context -> makeSession(state);

        LuaArgumentList args;
        session -> parseArguments(args);

        LuaValue *retValue = handler (module, methodName, args);
        if (retValue != NULL)
        {
            returnCount = session -> setReturnValue(retValue);
            //释放返回值
            retValue -> release();
        }

        //释放参数内存
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }

        //销毁会话
        context -> destorySession(session);
    }

    return returnCount;
}

void LuaModule::onRegister (const std::string &name, LuaContext *context)
{
    _name = name;
    _context = context;

    //注册模块
    lua_State *state = _context -> getMainSession() -> getState();
    LuaEngineAdapter::newTable(state);

    //设置模块名称。since ver 1.3
    LuaEngineAdapter::pushString(state, name.c_str());
    LuaEngineAdapter::setField(state, -2, "name");

    //写入模块标识
    LuaEngineAdapter::pushString(state, "module");
    LuaEngineAdapter::setField(state, -2, "_nativeType");

    LuaEngineAdapter::setGlobal(state, name.c_str());
}

void LuaModule::registerMethod(
        std::string methodName,
        LuaModuleMethodHandler handler)
{
    lua_State *state = getContext() -> getMainSession() -> getState();
    LuaEngineAdapter::getGlobal(state, getName().c_str());
    if (LuaEngineAdapter::isTable(state, -1))
    {
        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushString(state, methodName.c_str());
        LuaEngineAdapter::pushCClosure(state, methodRouteHandler, 2);

        LuaEngineAdapter::setField(state, -2, methodName.c_str());

        _methodMap[methodName] = handler;
    }

    LuaEngineAdapter::pop(state, 1);
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
