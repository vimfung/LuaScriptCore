//
// Created by 冯鸿杰 on 16/11/1.
//

#include <limits>
#include <stdio.h>
#include "LuaFunction.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaTuple.h"
#include "LuaNativeClass.hpp"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaObjectManager.h"

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaFunction);

/**
 * 方法种子，主要参与方法索引的生成，每次创建一个Function，该值就会自增。
 */
static int FunctionSeed = 0;

/**
 * 方法表名称
 */
static std::string FunctionsTableName = "_tmpFuncs_";

LuaFunction::LuaFunction ()
{
    
}

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
        char buf[40];
#if _WINDOWS
        sprintf_s(buf, sizeof(buf), "func_%ld", FunctionSeed);
#else
        snprintf(buf, sizeof(buf), "func_%d", FunctionSeed);
#endif
        _index = buf;
        FunctionSeed ++;
		FunctionSeed %= INT_MAX;

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

std::string LuaFunction::typeName()
{
    static std::string name = typeid(LuaFunction).name();
    return name;
}

LuaFunction::LuaFunction (LuaObjectDecoder *decoder)
    :LuaObject(decoder)
{
    int contextId = decoder -> readInt32();
    _context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(contextId));
    _index = decoder -> readString();
}

void LuaFunction::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    
    encoder -> writeInt32(_context -> objectId());
    encoder -> writeString(_index);
}

void cn::vimfung::luascriptcore::LuaFunction::push(LuaContext *context)
{
    lua_State *state = context -> getLuaState();

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
            //记录栈顶位置，用于计算返回值数量
            int top = lua_gettop(state);

            lua_getfield(state, -1, _index.c_str());
            if (lua_isfunction(state, -1))
            {
                int returnCount = 0;

                //初始化传递参数
                for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
                {
                    LuaValue *item = *i;
                    item->push(_context);
                }

                if (lua_pcall(state, (int)arguments -> size(), LUA_MULTRET, 0) == 0)
                {
                    //调用成功
                    returnCount = lua_gettop(state) - top;
                    if (returnCount > 1)
                    {
                        LuaTuple *tuple = new LuaTuple();
                        for (int i = 1; i <= returnCount; i++)
                        {
                            LuaValue *value = _context -> getValueByIndex(top + i);
                            tuple -> addReturnValue(value);
                            value -> release();
                        }

                        retValue = LuaValue::TupleValue(tuple);

                        tuple -> release();
                    }
                    else if (returnCount == 1)
                    {
                        retValue = _context -> getValueByIndex(-1);
                    }
                }
                else
                {
                    
                    //调用失败
                    returnCount = 1;

                    LuaValue *value = _context -> getValueByIndex(-1);
                    std::string errMessage = value -> toString();
                    _context -> raiseException(errMessage);

                    value -> release();
                }

                //弹出返回值
                lua_pop(state, returnCount);
            }
            else
            {
                //弹出function
                lua_pop(state, 1);
            }

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
