//
// Created by 冯鸿杰 on 16/11/1.
//

#include <sys/time.h>
#include <stdio.h>
#include "LuaFunction.h"
#include "LuaContext.h"
#include "LuaDefine.h"

/**
 * 方法种子，主要参与方法索引的生成，每次创建一个Function，该值就会自增。
 */
static int FunctionSeed = 0;

/**
 * 方法表名称
 */
static std::string FunctionsTableName = "_tmpFuncs_";

cn::vimfung::luascriptcore::LuaFunction::LuaFunction(LuaContext *context, int index)
{
    _context = context;

    lua_State *state = context -> getLuaState();
    if (index < 0)
    {
        //修正索引为负数情况
        index = lua_gettop(state) + index + 1;
    }

    lua_getglobal(state, "_G");
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, FunctionsTableName.c_str());
        if (lua_isnil(state, -1))
        {
            lua_pop(state, 1);

            //创建引用表
            lua_newtable(state);

            //放入全局变量_G中
            lua_pushvalue(state, -1);
            lua_setfield(state, -3, FunctionsTableName.c_str());
        }

        //根据时间戳创建索引
        struct timeval tv;
        gettimeofday(&tv,NULL);
        long timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000 + FunctionSeed;
        char buf[40];
        sprintf(buf, "func%ld", timestamp);
        _index = buf;
        FunctionSeed ++;

        lua_pushvalue(state, (int)index);
        lua_setfield(state, -2, _index.c_str());

        //弹出_tmpFuncs_
        lua_pop(state, 1);
    }

    //弹出_G
    lua_pop(state, 1);
}

cn::vimfung::luascriptcore::LuaFunction::~LuaFunction()
{
    if (!_index.empty())
    {
        //移除索引中的方法
        lua_State *state = _context -> getLuaState();

        lua_getglobal(state, "_G");
        if (lua_istable(state, -1))
        {
            lua_getfield(state, -1, FunctionsTableName.c_str());
            if (lua_istable(state, -1))
            {
                lua_pushnil(state);
                lua_setfield(state, -2, _index.c_str());
            }
            lua_pop(state, 1);

        }
        lua_pop(state, 1);
    }
}

void cn::vimfung::luascriptcore::LuaFunction::push()
{
    lua_State *state = _context -> getLuaState();

    lua_getglobal(state, "_G");
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, FunctionsTableName.c_str());
        if (lua_istable(state, -1))
        {
            lua_getfield(state, -1, _index.c_str());
        }
        else
        {
            lua_pushnil(state);
        }
        lua_remove(state, -2);

    }
    else
    {
        lua_pushnil(state);
    }
    lua_remove(state, -2);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaFunction::invoke(
        LuaArgumentList *arguments)
{
    lua_State *state = _context -> getLuaState();

    LuaValue *retValue = NULL;

    lua_getglobal(state, "_G");
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, FunctionsTableName.c_str());
        if (lua_istable(state, -1))
        {
            lua_getfield(state, -1, _index.c_str());
            if (lua_isfunction(state, -1))
            {
                //初始化传递参数
                for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
                {
                    LuaValue *item = *i;
                    item->push(_context);
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

            //弹出返回值
            lua_pop(state, 1);

        }

        //弹出_tmpFuncs_
        lua_pop(state, 1);

    }

    //弹出_G
    lua_pop(state, 1);

    if (!retValue)
    {
        retValue = new LuaValue();
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return retValue;
}