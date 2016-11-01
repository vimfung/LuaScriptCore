//
// Created by 冯鸿杰 on 16/11/1.
//

#include <sys/time.h>
#include <stdio.h>
#include "LuaFunction.h"
#include "LuaContext.h"
#include "LuaDefine.h"

static int FunctionIndex = 0;

cn::vimfung::luascriptcore::LuaFunction::LuaFunction(LuaContext *context, int index)
{
    _context = context;

    lua_State *state = context -> getLuaState();
    if (index < 0)
    {
        //修正索引为负数情况
        index = lua_gettop(state) + index + 1;
    }

    if (lua_istable(state, lua_upvalueindex(0)))
    {
        //根据时间戳创建索引
        struct timeval tv;
        gettimeofday(&tv,NULL);
        long timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000 + FunctionIndex;
        char buf[40];
        sprintf(buf, "func%ld", timestamp);
        _index = buf;
        FunctionIndex ++;

        lua_pushvalue(state, lua_upvalueindex(0));

        lua_pushvalue(state, index);
        lua_setfield(state, -2, _index.c_str());

        lua_pop(state, 1);
    }
}

cn::vimfung::luascriptcore::LuaFunction::~LuaFunction()
{
    if (!_index.empty())
    {
        //移除索引中的方法
        lua_State *state = _context -> getLuaState();
        if (lua_istable(state, lua_upvalueindex(0)))
        {
            lua_pushvalue(state, lua_upvalueindex(0));
            lua_pushnil(state);
            lua_setfield(state, -2, _index.c_str());
            lua_pop(state, 1);
        }
    }
}

void cn::vimfung::luascriptcore::LuaFunction::push()
{
    lua_State *state = _context -> getLuaState();
    if (lua_istable(state, lua_upvalueindex(0)))
    {
        lua_getfield(state, lua_upvalueindex(0), _index.c_str());
    }
    else
    {
        lua_pushnil(state);
    }
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaFunction::invoke(
        LuaArgumentList *arguments)
{
    lua_State *state = _context -> getLuaState();

    LuaValue *retValue = NULL;

    if (lua_istable(state, lua_upvalueindex(0)))
    {
        lua_pushvalue(state, lua_upvalueindex(0));
        lua_getfield(state, -1, _index.c_str());

        if (lua_isfunction(state, -1))
        {
            //初始化传递参数
            for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
            {
                LuaValue *item = *i;
                item->push(state);
            }

            if (lua_pcall(state, (int)arguments -> size(), 1, 0) == 0)
            {
                //调用成功
                retValue = _context -> getValueByIndex(-1);
            }
            else
            {
                //调用失败
                LuaValue *value = _context -> getValueByIndex(-1);
                std::string errMessage = value -> toString();
                _context -> raiseException(errMessage);

                value -> release();
            }
        }
        else
        {
            lua_pop(state, 1);
        }

        lua_pop(state, 1);
    }

    if (!retValue)
    {
        retValue = new LuaValue();
    }

    return retValue;
}