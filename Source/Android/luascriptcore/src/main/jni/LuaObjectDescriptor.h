//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAOBJECTDESC_H
#define ANDROID_LUAOBJECTDESC_H

#include "LuaObject.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            /**
             * Lua对象描述器
             */
            class LuaObjectDescriptor : public LuaObject
            {
            private:
                void *_object;

            public:
                LuaObjectDescriptor(void *object);
                ~LuaObjectDescriptor();

            public:

                /**
                 * 获取对象
                 */
                const void* getObject();
            };
        }
    }
}


#endif //ANDROID_LUAOBJECTDESC_H
