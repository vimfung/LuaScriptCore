//
// Created by 冯鸿杰 on 17/1/13.
//

#include "LuaObjectInstanceDescriptor.h"
#include "LuaObjectClass.h"

cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor::LuaObjectInstanceDescriptor ()
    : LuaObjectDescriptor(), _objectClass (NULL)
{

}

cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor::LuaObjectInstanceDescriptor(const void *instance, LuaObjectClass *objectClass)
    : LuaObjectDescriptor(instance), _objectClass(objectClass)
{

}

cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor::~LuaObjectInstanceDescriptor()
{
    _objectClass = NULL;
}

cn::vimfung::luascriptcore::modules::oo::LuaObjectClass* cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor::getObjectClass()
{
    return  _objectClass;
}

bool cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor::instanceOf (LuaObjectClass *objectClass)
{
    return this -> getObjectClass() -> subclassOf(objectClass);
}