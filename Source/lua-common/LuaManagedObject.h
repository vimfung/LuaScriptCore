//
// Created by 冯鸿杰 on 2017/5/12.
//

#ifndef ANDROID_LUAMANAGEDOBJECT_H
#define ANDROID_LUAMANAGEDOBJECT_H

#include <string>
#include "LuaObject.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;

            /**
             * 与Lua层有直接关系的对象，继承该类型可以使对象直接找对与其对应的Lua对象
             */
            class LuaManagedObject : public LuaObject
            {
            private:
                LuaContext *_context;

            protected:

                /**
                 * 交换层标识，每个对象应该保证唯一
                 */
                std::string _exchangeId;

            public:
                LuaManagedObject(LuaContext *context);
                LuaManagedObject (LuaObjectDecoder *decoder);

                /**
                 * 析构对象
                 */
                virtual ~LuaManagedObject();

            public:

                /**
                 * 获取上下文对象
                 * @return 上下文对象
                 */
                LuaContext* getContext();

            public:

                /**
                 * 获取交换层标识
                 */
                std::string getExchangeId();

                /**
                 * 入栈数据
                 */
                virtual void push(LuaContext *context);

                /**
                 序列化对象

                 @param encoder 编码器
                 */
                virtual void serialization (LuaObjectEncoder *encoder);
            };
        }
    }
}



#endif //ANDROID_LUAMANAGEDOBJECT_H
