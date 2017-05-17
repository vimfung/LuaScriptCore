//
// Created by 冯鸿杰 on 2017/5/12.
//

#include "LuaManagedObject.h"

using namespace cn::vimfung::luascriptcore;

LuaManagedObject::LuaManagedObject()
    : LuaObject()
{

}

LuaManagedObject::LuaManagedObject (LuaObjectDecoder *decoder)
    : LuaObject (decoder)
{

}

std::string LuaManagedObject::getLinkId()
{
    std::string linkId;
    return linkId;
}

void LuaManagedObject::push(LuaContext *context)
{

}

void LuaManagedObject::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
}