//
// Created by 冯鸿杰 on 16/9/30.
//

#include "LuaClassInstance.h"
#include "lua.hpp"
#include "../../../../../lua-core/src/lapi.h"

cn::vimfung::luascriptcore::modules::oo::LuaClassInstance::LuaClassInstance(LuaContext *context, int index)
{
    _context = context;
    _index = index;
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::modules::oo::LuaClassInstance::getField(std::string name)
{
    lua_State *state = _context -> getLuaState();

    lua_pushvalue(state, _index);

    lua_pushstring(state, name.c_str());
    lua_gettable(state, -2);

    LuaValue *retValue = _context -> getValueByIndex(-1);

    lua_settop(state, -3);

    return retValue;
}

void cn::vimfung::luascriptcore::modules::oo::LuaClassInstance::setField(std::string name, cn::vimfung::luascriptcore::LuaValue *value)
{
    lua_State *state = _context -> getLuaState();

    lua_pushvalue(state, _index);

    value -> push(state);
    lua_setfield(state, -2, name.c_str());

    lua_settop(state, -2);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::modules::oo::LuaClassInstance::callMethod(std::string methodName, LuaArgumentList *arguments)
{
    lua_State *state = _context -> getLuaState();

    lua_pushvalue(state, _index);

    LuaValue *resultValue = NULL;

    lua_getfield(state, -1, methodName.c_str());
    if (lua_isfunction(state, -1))
    {
        //如果为function则进行调用
        //第一个参数为实例引用
        lua_pushvalue(state, _index);

        for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
        {
            LuaValue *item = *i;
            item->push(state);
        }

        if (lua_pcall(state, (int)arguments -> size() + 1, 1, 0) == 0)
        {
            //调用成功
            resultValue = _context -> getValueByIndex(-1);
        }
    }

    lua_settop(state, -3);

    return resultValue;
}