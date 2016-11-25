//
// Created by vimfung on 16/8/29.
//

#include <stddef.h>
#include "LuaObjectManager.h"

cn::vimfung::luascriptcore::LuaObjectManager::LuaObjectManager()
{

}

cn::vimfung::luascriptcore::LuaObjectManager* cn::vimfung::luascriptcore::LuaObjectManager::SharedInstance()
{
    static LuaObjectManager *_manager = NULL;
    if (_manager == NULL)
    {
        _manager = new LuaObjectManager();
    }

    return _manager;
}

int cn::vimfung::luascriptcore::LuaObjectManager::putObject(LuaObject *object)
{
    object -> retain();

    _objectPool[object -> objectId()] = object;
    return object -> objectId();
}

void cn::vimfung::luascriptcore::LuaObjectManager::removeObject(int objectId)
{
    LuaObjectMap::iterator it = _objectPool.find(objectId);
    if (it != _objectPool.end())
    {
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