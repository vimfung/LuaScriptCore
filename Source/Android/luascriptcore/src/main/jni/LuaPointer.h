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
             * Userdata引用
             */
            typedef struct {

                void *value;

            }LuaUserdata, *LuaUserdataRef;

            /**
             * Lua的指针对象
             */
            class LuaPointer : public LuaObject
            {
            private:
                LuaUserdataRef _value;
                bool _needFree;

            public:
                LuaPointer (LuaUserdataRef userdata);
                LuaPointer (const void *value);
                ~LuaPointer();

            public:

                /**
                 * 获取指针值
                 */
                const LuaUserdataRef getValue();
            };

        }
    }
}


#endif //ANDROID_LUAPOINTER_H
