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
#include "StringUtils.h"
#include "LuaDataExchanger.h"
#include "LuaSession.h"
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaFunction);

LuaFunction::LuaFunction ()
    : LuaManagedObject()
{
    
}

LuaFunction::LuaFunction(LuaContext *context, int index)
    : LuaManagedObject()
{
    _context = context;
    _linkId = StringUtils::format("%p", this);

    _context -> getDataExchanger() -> setLuaObject(index, _linkId);

    LuaValue *value = LuaValue::FunctionValue(this);
    _context->retainValue(value);
    value -> release();
}

LuaFunction::~LuaFunction()
{
    if (_context != NULL)
    {
        LuaValue *value = LuaValue::FunctionValue(this);
        _context->releaseValue(value);
        value -> release();
    }
}

std::string LuaFunction::typeName()
{
    static std::string name = typeid(LuaFunction).name();
    return name;
}

LuaFunction::LuaFunction (LuaObjectDecoder *decoder)
    :LuaManagedObject(decoder)
{
    int contextId = decoder -> readInt32();
    _context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(contextId));
    _linkId = decoder -> readString();
}

void LuaFunction::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    
    encoder -> writeInt32(_context -> objectId());
    encoder -> writeString(_linkId);
}

void LuaFunction::push(LuaContext *context)
{
    
}

LuaValue* LuaFunction::invoke(LuaArgumentList *arguments)
{
    lua_State *state = _context -> getCurrentSession() -> getState();

    LuaValue *retValue = NULL;

    //记录栈顶位置，用于计算返回值数量
    int top = lua_gettop(state);
    _context -> getDataExchanger() -> getLuaObject(this);

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
                    LuaValue *value = LuaValue::ValueByIndex(_context, top + i);
                    tuple -> addReturnValue(value);
                    value -> release();
                }

                retValue = LuaValue::TupleValue(tuple);

                tuple -> release();
            }
            else if (returnCount == 1)
            {
                retValue = LuaValue::ValueByIndex(_context, -1);
            }
        }
        else
        {

            //调用失败
            returnCount = 1;

            LuaValue *value = LuaValue::ValueByIndex(_context, -1);
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


    if (!retValue)
    {
        retValue = new LuaValue();
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return retValue;
}


std::string LuaFunction::getLinkId()
{
    return _linkId;
}
