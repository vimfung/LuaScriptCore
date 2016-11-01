//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaObjectDescriptor.h"

cn::vimfung::luascriptcore::LuaObjectDescriptor::LuaObjectDescriptor(void *object)
{
    _object = object;
}

cn::vimfung::luascriptcore::LuaObjectDescriptor::~LuaObjectDescriptor()
{
    _object = NULL;
}

const void* cn::vimfung::luascriptcore::LuaObjectDescriptor::getObject()
{
    return _object;
}