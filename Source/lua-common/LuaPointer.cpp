//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include <stdlib.h>
#include <typeinfo>
#include "LuaPointer.h"
#include "LuaObjectDecoder.hpp"
#include "LuaObjectEncoder.hpp"
#include "LuaNativeClass.hpp"
#include "LuaEngineAdapter.hpp"
#include "StringUtils.h"
#include "LuaContext.h"
#include "LuaSession.h"
#include "LuaOperationQueue.h"
#include "lua.hpp"

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaPointer);

LuaPointer::LuaPointer (LuaContext *context)
    : LuaManagedObject(context)
{
    _needFree = false;
    _value = NULL;
}

LuaPointer::LuaPointer(LuaContext *context, LuaUserdataRef userdata)
    : LuaManagedObject(context)
{
    _needFree = false;
    _value = userdata;
    _exchangeId = StringUtils::format("%p", _value);
}

LuaPointer::LuaPointer(LuaContext *context, const void *value)
    : LuaManagedObject(context)
{
    _needFree = true;
    _value = (LuaUserdataRef)malloc(sizeof(LuaUserdataRef));
    _value -> value = (void *)value;
    _exchangeId = StringUtils::format("%p", _value);
}

LuaPointer::~LuaPointer()
{
    if (_needFree)
    {
        free(_value);
    }
    _value = NULL;
}

LuaPointer::LuaPointer (LuaObjectDecoder *decoder)
    : LuaManagedObject(decoder)
{
    void *objRef = NULL;
    objRef = (void *)decoder -> readInt64();
    
    _needFree = true;
    _value = (LuaUserdataRef)malloc(sizeof(LuaUserdataRef));
    _value -> value = (void *)objRef;
    
    _exchangeId = decoder -> readString();
}

std::string LuaPointer::typeName()
{
    static std::string name = typeid(LuaPointer).name();
    return name;
}

const LuaUserdataRef LuaPointer::getValue()
{
    return _value;
}

void LuaPointer::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    encoder -> writeInt64((long long)_value -> value);
    encoder -> writeString(_exchangeId);
}

void LuaPointer::push(LuaContext *context)
{
    context -> getOperationQueue() -> performAction([=](){

        lua_State *state = context -> getCurrentSession() -> getState();
        LuaEngineAdapter::pushLightUserdata(state, getValue());

    });
}

void LuaPointer::push(lua_State *state, LuaOperationQueue *queue)
{
    auto handler = [=](){

        LuaEngineAdapter::pushLightUserdata(state, getValue());

    };

    if (queue != NULL)
    {
        queue -> performAction(handler);
    }
    else
    {
        handler ();
    }
}
