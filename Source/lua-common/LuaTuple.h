//
// Created by 冯鸿杰 on 17/1/19.
//

#ifndef ANDROID_LUATUPLE_H
#define ANDROID_LUATUPLE_H

#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaObjectDecoder;
            class LuaObjectEncoder;

            /**
             * 元组
             */
            class LuaTuple : public LuaObject
            {
            private:
                LuaValueList _returnValues;

            public:
                /**
                 初始化
                 */
                LuaTuple ();
                
                /**
                 * 初始化, 在反序列化对象时会触发该方法
                 *
                 * @param decoder 解码器
                 */
                LuaTuple (LuaObjectDecoder *decoder);
                
                /**
                 销毁对象
                 */
                ~LuaTuple();
                
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
                 * 获取返回值数量
                 */
                long count();

                /**
                 * 添加返回值
                 *
                 * @param value 返回值
                 */
                void addReturnValue(LuaValue *value);

                /**
                 * 获取返回值
                 *
                 * @param index 索引
                 *
                 * @return 返回值
                 */
                LuaValue *getReturnValueByIndex(int index);

            public:

                /**
                 * 入栈数据
                 *
                 * @param context 上下文对象
                 */
                void push(LuaContext *context);
            };

        }
    }
}


#endif //ANDROID_LUATUPLE_H
