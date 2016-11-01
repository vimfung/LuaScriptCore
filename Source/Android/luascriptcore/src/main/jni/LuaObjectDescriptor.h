//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAOBJECTDESC_H
#define ANDROID_LUAOBJECTDESC_H

#include "LuaObject.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;

            /**
             * Lua对象描述器
             */
            class LuaObjectDescriptor : public LuaObject
            {
            private:
                void *_object;
                void *_userData;

            protected:

                /**
                 * 设置对象
                 *
                 * @param object 对象引用
                 */
                void setObject(const void *object);

            public:

                /**
                 * 初始化对象描述器
                 */
                LuaObjectDescriptor();

                /**
                 * 初始化对象描述器
                 *
                 * @param object 对象指针
                 */
                LuaObjectDescriptor(const void *object);

                /**
                 * 释放对象描述器
                 */
                virtual ~LuaObjectDescriptor();

            public:

                /**
                 * 设置自定义数据
                 *
                 * @param userdata 数据对象
                 */
                void setUserdata(void *userdata);

                /**
                 * 获取自定义数据
                 *
                 * @return 数据对象
                 */
                const void* getUserdata();

                /**
                 * 获取对象
                 *
                 * @return 对象引用
                 */
                const void* getObject();

                /**
                 * 入栈数据
                 *
                 * @param context 上下文对象
                 */
                virtual void push(LuaContext *context);
            };
        }
    }
}


#endif //ANDROID_LUAOBJECTDESC_H
