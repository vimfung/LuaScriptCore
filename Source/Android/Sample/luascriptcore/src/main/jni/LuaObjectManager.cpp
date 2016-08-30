//
// Created by vimfung on 16/8/29.
//

#include <stddef.h>
#include "LuaObjectManager.h"
#include "LuaDefine.h"

cn::vimfung::luascriptcore::LuaObjectManager::LuaObjectManager()
{
    _objectSeqId = 0;
}

cn::vimfung::luascriptcore::LuaObjectManager* cn::vimfung::luascriptcore::LuaObjectManager::SharedInstance()
{
    static LuaObjectManager *_manager = NULL;
    if (_manager == NULL)
    {
        LOGI("create object manager");
        _manager = new LuaObjectManager();
    }

    return _manager;
}

int cn::vimfung::luascriptcore::LuaObjectManager::putObject(LuaObject *object)
{
    object -> retain();

    _objectSeqId++;
    _objectPool[_objectSeqId] = object;
    return _objectSeqId;
}

void cn::vimfung::luascriptcore::LuaObjectManager::removeObject(int objectId)
{
    LuaObjectMap::iterator it = _objectPool.find(objectId);
    if (it != _objectPool.end())
    {
        LOGI("release object %d", objectId);
        it -> second -> release();
        _objectPool.erase(it);
    }
}

cn::vimfung::luascriptcore::LuaObject* cn::vimfung::luascriptcore::LuaObjectManager::getObject(int objectId)
{
    LuaObjectMap::iterator it = _objectPool.find(objectId);
    if (it != _objectPool.end())
    {
        return it -> second;
    }

    return NULL;
}