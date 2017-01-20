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
             * 方法对象, 表示了一个对应在Lua中的function，在lua中传入一个function到本地方法，将会自动转换为此类型的实例对象。
             */
            class LuaFunction : public LuaObject
            {
            private:

                /**
                 * 方法索引
                 */
                std::string _index;

                /**
                 * Lua上下文环境
                 */
                LuaContext *_context;

            public:

                /**
                 * 初始化方法对象
                 *
                 * @param context 上下文环境
                 * @param index 数据栈索引
                 */
                LuaFunction (LuaContext *context, int index);

                /**
                 * 销毁方法对象
                 */
                ~LuaFunction();

            public:

                /**
                 * 入栈数据
                 */
                void push(LuaContext *context);

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
