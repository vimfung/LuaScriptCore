//
// Created by 冯鸿杰 on 2018/12/25.
//

#ifndef ANDROID_LUATHREAD_H
#define ANDROID_LUATHREAD_H


#include "lua.h"
#include "LuaObject.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;

            /**
             * 线程
             */
            class LuaCoroutine : public LuaObject
            {
            private:
                LuaContext *_context;
                std::string _exchangeId;
                lua_State *_state;

            public:

                /**
                 * 初始化
                 * @param context 上下文对象
                 * @param handler 线程执行方法
                 */
                LuaCoroutine(LuaContext *context);

                /**
                 * 释放对象
                 */
                ~LuaCoroutine();

            public:

                /**
                 * 获取上下文对象
                 * @return 上下文对象
                 */
                LuaContext* getContext();

                /**
                 * 获取状态
                 * @return 状态对象
                 */
                lua_State* getState();

            };
        }
    }
}


#endif //ANDROID_LUATHREAD_H
