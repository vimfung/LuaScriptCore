//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaPointer.h"

cn::vimfung::luascriptcore::LuaPointer::LuaPointer(const void *value)
{
    _value = (void *)value;
}

cn::vimfung::luascriptcore::LuaPointer::~LuaPointer()
{
    _value = NULL;
}

const void* cn::vimfung::luascriptcore::LuaPointer::getValue()
{
    return _value;
}