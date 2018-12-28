//
// Created by 冯鸿杰 on 2018/12/25.
//

#ifndef ANDROID_LUATHREAD_H
#define ANDROID_LUATHREAD_H


#include "LuaContext.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            /**
             * 线程
             */
            class LuaThread : public LuaContext
            {
            private:
                LuaContext *_context;
                bool _finished;
                std::string _exchangeId;

            public:

                /**
                 * 初始化
                 * @param context 上下文对象
                 * @param handler 线程执行方法
                 */
                LuaThread(LuaContext *context, LuaMethodHandler handler);

                /**
                 * 释放对象
                 */
                ~LuaThread();

            public:

                /**
                 * 获取上下文对象
                 * @return 上下文对象
                 */
                LuaContext* getContext();

                /**
                 * 判断线程是否执行完成
                 * @return true 完成，false 未完成
                 */
                bool hasFinished();

                /**
                 * 恢复线程
                 * @param argumentList 参数列表
                 */
                void resume(LuaArgumentList argumentList);

                /**
                 * 挂起线程
                 * @param resultValue 返回值
                 */
                void yield(LuaValue *resultValue);

            };
        }
    }
}


#endif //ANDROID_LUATHREAD_H
