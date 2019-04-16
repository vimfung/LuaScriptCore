//
// Created by 冯鸿杰 on 2019/3/11.
//

#ifndef ANDROID_LUAERROR_H
#define ANDROID_LUAERROR_H

#include "LuaObject.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaSession;

            /**
             * 异常信息类
             */
            class LuaError : public LuaObject
            {
            private:

                /**
                 * 错误消息
                 */
                std::string _message;

                /**
                 * 会话对象
                 */
                LuaSession *_session;

            public:

                /**
                 * 初始化
                 *
                 * @param session 会话对象
                 * @param message 错误消息
                 */
                LuaError(LuaSession *session, std::string const& message);

                /**
                 * 析构对象
                 */
                virtual ~LuaError();

            public:

                /**
                 * 获取错误消息
                 * @return 错误消息
                 */
                std::string getMessage();

                /**
                 * 获取会话
                 * @return 会话对象
                 */
                LuaSession *getSession();
            };

        }
    }
}


#endif //ANDROID_LUAERROR_H
