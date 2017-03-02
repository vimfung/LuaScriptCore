//
// Created by 冯鸿杰 on 17/1/13.
//

#include "LuaObjectInstanceDescriptor.h"
#include "LuaObjectClass.h"
#include "LuaNativeClass.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaObjectEncoder.hpp"
#include "LuaContext.h"

using namespace cn::vimfung::luascriptcore;
using namespace cn::vimfung::luascriptcore::modules::oo;

DECLARE_NATIVE_CLASS(LuaObjectInstanceDescriptor);

LuaObjectInstanceDescriptor::LuaObjectInstanceDescriptor ()
    : LuaObjectDescriptor(), _objectClass (NULL)
{

}

LuaObjectInstanceDescriptor::LuaObjectInstanceDescriptor(const void *instance, LuaObjectClass *objectClass)
    : LuaObjectDescriptor(instance), _objectClass(objectClass)
{
    
}

LuaObjectInstanceDescriptor::LuaObjectInstanceDescriptor (LuaObjectDecoder *decoder)
    : LuaObjectDescriptor(decoder)
{
    std::string clsName = decoder -> readString();
    LuaModule *cls = decoder -> getContext() -> getModule(clsName);
    _objectClass = (LuaObjectClass *)cls;
}

LuaObjectInstanceDescriptor::~LuaObjectInstanceDescriptor()
{
    _objectClass = NULL;
}

std::string LuaObjectInstanceDescriptor::typeName()
{
    static std::string name = typeid(LuaObjectInstanceDescriptor).name();
    return name;
}

LuaObjectClass* LuaObjectInstanceDescriptor::getObjectClass()
{
    return  _objectClass;
}

bool LuaObjectInstanceDescriptor::instanceOf (LuaObjectClass *objectClass)
{
    return this -> getObjectClass() -> subclassOf(objectClass);
}

void LuaObjectInstanceDescriptor::serialization (LuaObjectEncoder *encoder)
{
    LuaObjectDescriptor::serialization(encoder);
    
    //写入类型名称
    encoder -> writeString(this -> getObjectClass() -> getName());
}

void LuaObjectInstanceDescriptor::push(LuaContext *context)
{
    this -> getObjectClass() -> push(this);
}
