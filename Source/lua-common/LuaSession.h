//
// Created by 冯鸿杰 on 2017/7/5.
//

#ifndef ANDROID_LUASESSION_H
#define ANDROID_LUASESSION_H

#include "lua.hpp"
#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaValue;

            /**
             * 会话，从Lua调用原生方法时依靠此会话来处理参数和返回值
             */
            class LuaSession : public LuaObject
            {
            private:
                
                lua_State *_state;
                LuaContext *_context;
                
            public:

                /**
                 * 初始化
                 *
                 * @param state 状态
                 * @param context 上下文对象
                 */
                LuaSession(lua_State *state, LuaContext *context);

                /**
                 * 销毁对象
                 */
                virtual ~LuaSession();

            public:

                /**
                 * 获取Lua状态
                 *
                 * @return Lua状态
                 */
                lua_State* getState();

                /**
                 * 获取上下文对象
                 *
                 * @return 上下文对象
                 */
                LuaContext* getContext();
                
                /**
                 上一个会话
                 */
                LuaSession* prevSession;

            public:

                /**
                 * 解析参数
                 *
                 * @param argumentList 参数列表
                 */
                void parseArguments(LuaArgumentList &argumentList);

                /**
                 * 设置返回值
                 *
                 * @param value 返回值
                 *
                 * @return 返回值数量
                 */
                int setReturnValue(LuaValue *value);
            };
        }
    }
}


#endif //ANDROID_LUASESSION_H
