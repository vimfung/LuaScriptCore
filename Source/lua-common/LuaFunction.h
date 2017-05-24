//
// Created by 冯鸿杰 on 16/11/1.
//

#ifndef ANDROID_LUAFUNCTION_H
#define ANDROID_LUAFUNCTION_H

#include "LuaManagedObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaValue;
            class LuaObjectDecoder;
            class LuaObjectEncoder;

            /**
             * 方法对象, 表示了一个对应在Lua中的function，在lua中传入一个function到本地方法，将会自动转换为此类型的实例对象。
             */
            class LuaFunction : public LuaManagedObject
            {
            private:

                /**
                 * Lua上下文环境
                 */
                LuaContext *_context;

                /**
                 * 连接ID
                 */
                std::string _linkId;

            public:
                
                /**
                 * 初始化方法对象
                 */
                LuaFunction ();
                
                /**
                 * 初始化方法对象
                 *
                 * @param context 上下文环境
                 * @param index 数据栈索引
                 */
                LuaFunction (LuaContext *context, int index);
                
                /**
                 * 初始化, 在反序列化对象时会触发该方法
                 *
                 * @param decoder 解码器
                 */
                LuaFunction (LuaObjectDecoder *decoder);

                /**
                 * 释放对象
                 */
                ~LuaFunction();
                
                /**
                 获取类型名称
                 
                 @return 类型名称
                 */
                virtual std::string typeName();
                
                /**
                 序列化对象
                 
                 @param encoder 编码器
                 */
                virtual void serialization (LuaObjectEncoder *encoder);

            public:

                /**
                 * 调用方法
                 *
                 * @param arguments 参数列表
                 *
                 * @return 返回值
                 */
                LuaValue* invoke(LuaArgumentList *arguments);

            public:

                /**
                 * 获取对象标识
                 *
                 * @return 对象标识
                 */
                virtual std::string getLinkId();

                /**
                 * 入栈数据
                 */
                virtual void push(LuaContext *context);
            };

        }
    }
}


#endif //ANDROID_LUAFUNCTION_H
