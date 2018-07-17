//
// Created by 冯鸿杰 on 2018/7/2.
//

#ifndef ANDROID_LUAOPERATIONQUEUE_H
#define ANDROID_LUAOPERATIONQUEUE_H

#include "LuaObject.h"
#include <mutex>
#include <functional>
#include <thread>

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            /**
             * 操作队列
             */
            class LuaOperationQueue : public LuaObject
            {

            public:

                /**
                 * 执行操作
                 * @param action 操作内容
                 */
                void performAction(std::function<void(void)> const& action);

            private:

                /**
                 * 当前线程ID
                 */
                std::thread::id _curThreadId;

                /**
                 * 线程锁
                 */
                std::mutex _threadLocker;
            };
        }
    }
}


#endif //ANDROID_LUAOPERATIONQUEUE_H
