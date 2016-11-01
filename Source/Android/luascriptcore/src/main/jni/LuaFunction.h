//
// Created by 冯鸿杰 on 16/11/1.
//

#ifndef ANDROID_LUAFUNCTION_H
#define ANDROID_LUAFUNCTION_H

#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaValue;

            /**
             * 方法对象
             */
            class LuaFunction : public LuaObject
            {
            private:
                std::string _index;
                LuaContext *_context;

            public:
                LuaFunction (LuaContext *context, int index);
                ~LuaFunction();

            public:
                /**
                 * 入栈数据
                 */
                void push();

                /**
                 * 调用方法
                 *
                 * @param arguments 参数列表
                 *
                 * @return 返回值
                 */
                LuaValue* invoke(LuaArgumentList *arguments);
            };

        }
    }
}


#endif //ANDROID_LUAFUNCTION_H
