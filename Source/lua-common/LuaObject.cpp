//
// Created by vimfung on 16/8/23.
//

#include "LuaObject.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaObjectManager.h"
#include <map>
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;



static int _objSeqId = 0;

LuaObject::LuaObject()
{
    _retainCount = 1;

    _objSeqId ++;
    _objectId = _objSeqId;
}

LuaObject::LuaObject (LuaObjectDecoder *decoder)
{
    _retainCount = 1;
    
    _objectId = decoder -> readInt32();
    if (_objectId == 0)
    {
        //分配对象标识
        _objSeqId ++;
        _objectId = _objSeqId;
    }
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

std::string LuaObject::typeName()
{
    static std::string name = typeid(LuaObject).name();
    return name;
}

void LuaObject::serialization (LuaObjectEncoder *encoder)
{
    encoder -> writeInt32(_objectId);
}
