//
// Created by vimfung on 16/8/23.
//

#include "LuaObject.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include <map>
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;

typedef std::map<std::string, std::string> MappingClassesMap;

static int _objSeqId = 0;
static MappingClassesMap _mappingClassesMap;

LuaObject::LuaObject()
{
    _retainCount = 1;

    _objSeqId ++;
    _objectId = _objSeqId;
}

LuaObject::LuaObject (LuaObjectDecoder *decoder)
{
    _retainCount = 1;
    
    _objSeqId ++;
    _objectId = _objSeqId;
}

LuaObject::~LuaObject()
{

}

int LuaObject::objectId()
{
    return _objectId;
}

void LuaObject::retain()
{
    _retainCount ++;
}

void LuaObject::release()
{
    _retainCount --;
    if (_retainCount <= 0)
    {
        delete this;
    }
}

void LuaObject::setMappingClassType(std::string className, std::string mappingClassName)
{
    _mappingClassesMap[className] = mappingClassName;
}

void LuaObject::serialization (std::string className, LuaObjectEncoder *encoder)
{
    if (className.empty())
    {
        className = typeid(LuaObject).name();
    }
    
    MappingClassesMap::iterator it = _mappingClassesMap.find(className);
    if (it != _mappingClassesMap.end())
    {
        encoder -> writeByte('L');
        encoder -> writeString(it -> second);
        encoder -> writeByte(';');
    }
    else
    {
        encoder -> writeByte('L');
        encoder -> writeString(className);
        encoder -> writeByte(';');
    }
}
