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

typedef std::map<int, LuaObject *> ObjectPoolMap;

/**
 对象流水号
 */
static int _objSeqId = 0;

/**
 对象池
 */
static ObjectPoolMap _objectPool;

LuaObject::LuaObject()
{
    _retainCount = 1;

    _objSeqId ++;
    _objectId = _objSeqId;
    
    _objectPool[_objectId] = this;
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
    
    _objectPool[_objectId] = this;
}

LuaObject::~LuaObject()
{
    ObjectPoolMap::iterator it = _objectPool.find(_objectId);
    if (it != _objectPool.end())
    {
        _objectPool.erase(it);
    }
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

LuaObject* LuaObject::findObject(int objectId)
{
    ObjectPoolMap::iterator it = _objectPool.find(objectId);
    
    if (it != _objectPool.end())
    {
        return it -> second;
    }
    
    return NULL;
}
