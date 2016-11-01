//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAPOINTER_H
#define ANDROID_LUAPOINTER_H

#include "LuaObject.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            /**
             * Lua的指针对象
             */
            class LuaPointer : public LuaObject
            {
            private:
                void *_value;

            public:
                LuaPointer (const void *value);
                ~LuaPointer();

            public:

                /**
                 * 获取指针值
                 */
                const void* getValue();
            };

        }
    }
}


#endif //ANDROID_LUAPOINTER_H
