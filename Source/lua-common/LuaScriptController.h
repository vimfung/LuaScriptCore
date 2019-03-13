//
// Created by 冯鸿杰 on 2019/3/12.
//

#ifndef ANDROID_LUARUNSCRIPTCONFIG_H
#define ANDROID_LUARUNSCRIPTCONFIG_H

#include "LuaObject.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {

            /**
             * 执行脚本配置
             */
            class LuaScriptController : public LuaObject
            {
            public:

                /**
                 * 执行超时时间(单位：秒)，当脚本执行大于当前值时则会强制退出执行，默认为0，表示没有超时
                 */
                int timeout;

                /**
                 * 是否强制退出脚本执行, 内部使用
                 */
                bool isForceExit;

                /**
                 * 脚本开始执行时间, 内部计时使用
                 */
                int64_t startTime;

            public:

                /**
                 * 初始化
                 */
                LuaScriptController();

            public:

                /**
                 * 设置脚本执行超时时间
                 * @param timeout 超时时间，0 表示不限制时长
                 */
                void setTimeout(int timeout);

                /**
                 * 强制退出脚本执行
                 */
                void forceExit();
            };
        }
    }
}

#endif //ANDROID_LUARUNSCRIPTCONFIG_H
