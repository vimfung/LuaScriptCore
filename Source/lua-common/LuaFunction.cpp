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
#include "LuaEngineAdapter.hpp"
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

    int errFuncIndex = _context -> catchException();
    //记录栈顶位置，用于计算返回值数量
    int top = LuaEngineAdapter::getTop(state);
    _context -> getDataExchanger() -> getLuaObject(this);

    if (LuaEngineAdapter::isFunction(state, -1))
    {
        int returnCount = 0;

        //初始化传递参数
        for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
        {
            LuaValue *item = *i;
            item->push(_context);
        }

        if (LuaEngineAdapter::pCall(state, (int)arguments -> size(), LUA_MULTRET, errFuncIndex) == 0)
        {
            //调用成功
            returnCount = LuaEngineAdapter::getTop(state) - top;
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
            returnCount = LuaEngineAdapter::getTop(state) - top;
        }

        //弹出返回值
        LuaEngineAdapter::pop(state, returnCount);
    }
    else
    {
        //弹出function
        LuaEngineAdapter::pop(state, 1);
    }
    
    //移除异常捕获方法
    LuaEngineAdapter::remove(state, errFuncIndex);

    if (!retValue)
    {
        retValue = new LuaValue();
    }

    //回收内存
    _context -> gc();

    return retValue;
}


std::string LuaFunction::getLinkId()
{
    return _linkId;
}
