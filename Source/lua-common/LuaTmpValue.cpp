//
//  LuaTmpValue.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/12/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaTmpValue.hpp"
#include "LuaContext.h"
#include "LuaSession.h"
#include "LuaEngineAdapter.hpp"

using namespace cn::vimfung::luascriptcore;

LuaTmpValue::LuaTmpValue(LuaContext *context, int index)
{
    lua_State *state = context -> getCurrentSession() -> getState();
    _context = context;
    _index = LuaEngineAdapter::absIndex(state, index);
    _parsedValue = NULL;
}

LuaTmpValue::~LuaTmpValue()
{
    if (_parsedValue != NULL)
    {
        _parsedValue -> release();
        _parsedValue = NULL;
    }
}

void LuaTmpValue::_parseValue ()
{
    if (_parsedValue == NULL)
    {
        _parsedValue = LuaValue::ValueByIndex(_context, _index);
    }
}

LuaValueType LuaTmpValue::getType()
{
    _parseValue();
    return _parsedValue -> getType();
}

lua_Integer LuaTmpValue::toInteger()
{
    _parseValue();
    return _parsedValue -> toInteger();
}

const std::string LuaTmpValue::toString()
{
    _parseValue();
    return _parsedValue -> toString();
}

double LuaTmpValue::toNumber()
{
    _parseValue();
    return _parsedValue -> toNumber();
}

bool LuaTmpValue::toBoolean()
{
    _parseValue();
    return _parsedValue -> toBoolean();
}

const char* LuaTmpValue::toData()
{
    _parseValue();
    return _parsedValue -> toData();
}

size_t LuaTmpValue::getDataLength()
{
    _parseValue();
    return _parsedValue -> getDataLength();
}

LuaValueList* LuaTmpValue::toArray()
{
    _parseValue();
    return _parsedValue -> toArray();
}

LuaValueMap* LuaTmpValue::toMap()
{
    _parseValue();
    return _parsedValue -> toMap();
}

LuaPointer* LuaTmpValue::toPointer()
{
    _parseValue();
    return _parsedValue -> toPointer();
}

LuaFunction* LuaTmpValue::toFunction()
{
    _parseValue();
    return _parsedValue -> toFunction();
}

LuaTuple* LuaTmpValue::toTuple()
{
    _parseValue();
    return _parsedValue -> toTuple();
}

LuaObjectDescriptor* LuaTmpValue::toObject()
{
    _parseValue();
    return _parsedValue -> toObject();
}

LuaExportTypeDescriptor* LuaTmpValue::toType()
{
    _parseValue();
    return _parsedValue -> toType();
}

void LuaTmpValue::push(LuaContext *context)
{
    if (_parsedValue != NULL)
    {
        _parsedValue -> push(context);
    }
    else
    {
        lua_State *state = _context -> getCurrentSession() -> getState();
        LuaEngineAdapter::pushValue(state, _index);
    }
}

void LuaTmpValue::serialization (LuaObjectEncoder *encoder)
{
    _parseValue();
    _parsedValue -> serialization(encoder);
}
