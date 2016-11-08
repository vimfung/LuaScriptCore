//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include <malloc.h>
#include "LuaPointer.h"

cn::vimfung::luascriptcore::LuaPointer::LuaPointer(cn::vimfung::luascriptcore::LuaUserdataRef userdata)
{
    _needFree = false;
    _value = userdata;
}

cn::vimfung::luascriptcore::LuaPointer::LuaPointer(const void *value)
{
    _needFree = true;
    _value = (LuaUserdataRef)malloc(sizeof(LuaUserdataRef));
    _value -> value = (void *)value;
}

cn::vimfung::luascriptcore::LuaPointer::~LuaPointer()
{
    if (_needFree)
    {
        free(_value);
    }
    _value = NULL;
}

const cn::vimfung::luascriptcore::LuaUserdataRef cn::vimfung::luascriptcore::LuaPointer::getValue()
{
    return _value;
}