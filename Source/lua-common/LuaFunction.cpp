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
#include "LuaOperationQueue.h"
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaFunction);

LuaFunction::LuaFunction (LuaContext *context)
    : LuaManagedObject(context)
{
    
}

LuaFunction::LuaFunction(LuaContext *context, int index)
    : LuaManagedObject(context)
{
    _exchangeId = StringUtils::format("%p", this);

    getContext() -> getDataExchanger() -> setLuaObject(index, _exchangeId);
    getContext() -> getDataExchanger() -> retainLuaObject(this);
}

LuaFunction::~LuaFunction()
{
    if (getContext() != NULL)
    {
        getContext() -> getDataExchanger() -> releaseLuaObject(this);
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
    decoder -> readInt32(); //读取Unity中传过来的ContextID，并忽略(decoder中包含context对象)
    _exchangeId = decoder -> readString();
}

void LuaFunction::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    encoder -> writeInt32(getContext() -> objectId());
    encoder -> writeString(_exchangeId);
}

void LuaFunction::push(LuaContext *context)
{
    
}

void LuaFunction::push(lua_State *state, LuaOperationQueue *queue)
{

}

LuaValue* LuaFunction::invoke(LuaArgumentList *arguments)
{
    return LuaFunction::invoke(arguments, NULL);
}


LuaValue* LuaFunction::invoke(LuaArgumentList *arguments, LuaScriptController *scriptController)
{
    LuaValue *retValue = NULL;

    getContext() -> getOperationQueue() -> performAction([=, &retValue]() {

        LuaSession *session = getContext() -> getCurrentSession();
        lua_State *state = session -> getState();

        session->setScriptController(scriptController);

        int errFuncIndex = getContext() -> catchException();
        //记录栈顶位置，用于计算返回值数量
        int top = LuaEngineAdapter::getTop(state);
        getContext() -> getDataExchanger() -> getLuaObject(this);

        if (LuaEngineAdapter::isFunction(state, -1))
        {
            int returnCount = 0;

            //初始化传递参数
            for (LuaArgumentList::iterator i = arguments -> begin(); i != arguments -> end() ; ++i)
            {
                LuaValue *item = *i;
                item->push(getContext());
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
                        LuaValue *value = LuaValue::ValueByIndex(getContext(), top + i);
                        tuple -> addReturnValue(value);
                        value -> release();
                    }

                    retValue = LuaValue::TupleValue(tuple);

                    tuple -> release();
                }
                else if (returnCount == 1)
                {
                    retValue = LuaValue::ValueByIndex(getContext(), -1);
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
        getContext() -> gc();

        session -> setScriptController(NULL);

    });


    return retValue;
}