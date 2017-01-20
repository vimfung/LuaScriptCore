//
// Created by vimfung on 16/8/29.
//

#ifndef SAMPLE_LUAOBJECTMANAGER_H
#define SAMPLE_LUAOBJECTMANAGER_H

#include <map>
#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {

            class LuaObjectManager
            {
            private:
                LuaObjectManager();

            public:
                /**
                 * 获取共享的对象管理实例
                 */
                static LuaObjectManager* SharedInstance();

            private:
                LuaObjectMap _objectPool;

            public:
                int putObject(LuaObject *object);
                void removeObject(int objectId);
                LuaObject* getObject(int objectId);
            };
        }
    }
}

#endif //SAMPLE_LUAOBJECTMANAGER_H
