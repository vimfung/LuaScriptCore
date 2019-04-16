//
// Created by 冯鸿杰 on 2018/12/25.
//

#ifndef ANDROID_LUATHREAD_H
#define ANDROID_LUATHREAD_H

#include "lua.h"
#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaFunction;
            class LuaScriptController;

            /**
             * 线程
             */
            class LuaCoroutine : public LuaObject
            {
            private:
                LuaContext *_context;
                std::string _exchangeId;
                lua_State *_state;
                LuaScriptController *_scriptController;   //脚本控制器

            public:

                /**
                 * 初始化
                 * @param context 上下文对象
                 */
                LuaCoroutine(LuaContext *context);

                /**
                 * 释放对象
                 */
                virtual ~LuaCoroutine();

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

                /**
                 * 获取脚本控制器
                 * @return 脚本控制器
                 */
                LuaScriptController* getScriptController();

                /**
                 * 设置脚本控制器
                 * @param scriptController 脚本控制器，设置为NULL时表示清空控制器
                 */
                void setScriptController(LuaScriptController *scriptController);

                /**
                 * 执行协程
                 * @param handler 事件处理器
                 * @param arguments 参数列表
                 * @param scriptController 脚本控制器
                 */
                void run(LuaFunction *handler, LuaArgumentList arguments, LuaScriptController *scriptController);
            };
        }
    }
}


#endif //ANDROID_LUATHREAD_H
