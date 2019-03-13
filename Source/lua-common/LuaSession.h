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
            class LuaError;
            class LuaScriptController;

            /**
             * 会话，从Lua调用原生方法时依靠此会话来处理参数和返回值
             */
            class LuaSession : public LuaObject
            {
            private:
                
                lua_State *_state;
                LuaContext *_context;
                bool _lightweight;
                LuaError *_lastError;           //最后一次异常信息
                LuaScriptController *_scriptController;   //脚本控制器

            public:

                /**
                 * 初始化
                 *
                 * @param state 状态
                 * @param context 上下文对象
                 * @param lightweight 是否为轻量级会话，ture时表示绘画销毁时不需要进行内存回收，否则需要
                 */
                LuaSession(lua_State *state, LuaContext *context, bool lightweight);

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

                /**
                 * 获取错误信息
                 * @return true 错误，false 执行正常
                 */
                LuaError* getLastError();

                /**
                 * 获取脚本控制器
                 * @return 脚本控制器
                 */
                LuaScriptController* getScriptController();

                /**
                 * 清除错误
                 */
                void clearError();

            public:

                /**
                 * 解析参数
                 *
                 * @param argumentList 参数列表
                 */
                void parseArguments(LuaArgumentList &argumentList);

                /**
                 * 解析参数
                 *
                 * @param argumentList 参数列表
                 * @param fromIndex 从哪个索引开始解析
                 */
                void parseArguments(LuaArgumentList &argumentList, int fromIndex);

                /**
                 * 设置返回值
                 *
                 * @param value 返回值
                 *
                 * @return 返回值数量
                 */
                int setReturnValue(LuaValue *value);

                /**
                 * 报告Lua异常，与checkException配合使用可以让Lua中断执行
                 * @param message 异常描述消息
                 */
                void reportLuaException(std::string const& message);

                /**
                 * 设置脚本控制器
                 * @param scriptController 脚本控制器，传入NULL表示清空脚本控制器
                 */
                void setScriptController(LuaScriptController *scriptController);

            };
        }
    }
}


#endif //ANDROID_LUASESSION_H
