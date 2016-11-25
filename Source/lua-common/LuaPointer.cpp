//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include <stdlib.h>
#include <typeinfo>
#include "LuaPointer.h"

using namespace cn::vimfung::luascriptcore;

LuaPointer::LuaPointer(LuaUserdataRef userdata)
{
    _needFree = false;
    _value = userdata;
}

LuaPointer::LuaPointer(const void *value)
{
    _needFree = true;
    _value = (LuaUserdataRef)malloc(sizeof(LuaUserdataRef));
    _value -> value = (void *)value;
}

LuaPointer::~LuaPointer()
{
    if (_needFree)
    {
        free(_value);
    }
    _value = NULL;
}

const LuaUserdataRef LuaPointer::getValue()
{
    return _value;
}

void LuaPointer::serialization (std::string className, LuaObjectEncoder *encoder)
{
    if (className.empty())
    {
        className = typeid(LuaPointer).name();
    }
    
    LuaObject::serialization(className, encoder);
}
