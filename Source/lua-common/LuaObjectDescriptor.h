//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAOBJECTDESC_H
#define ANDROID_LUAOBJECTDESC_H

#include "LuaObject.h"
#include <string>

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

                /**
                 * 对象
                 */
                void *_object;

                /**
                 * 引用标识，每个实例对象在创建lua实例时都会向_G表的_instanceRefs_表写入一个引用，方便查找对应的引用对象，而改表对应引用的key就是该属性的值。
                 */
                std::string _refId;

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
                
                /**
                 序列化对象
                 
                 @param className 类型名称
                 @param encoder 编码器
                 */
                virtual void serialization (std::string className, LuaObjectEncoder *encoder);

            public:

                /**
                 * 获取对象
                 *
                 * @return 对象引用
                 */
                const void* getObject();

                /**
                 * 获取引用标识
                 */
                void setReferenceId(const std::string &refId);

                /**
                 * 设置引用标识
                 */
                std::string getReferenceId();

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
