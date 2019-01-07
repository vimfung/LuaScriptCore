//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAPOINTER_H
#define ANDROID_LUAPOINTER_H

#include "LuaObject.h"
#include "LuaManagedObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {
            
            class LuaObjectDecoder;
            class LuaObjectEncoder;

            /**
             * Lua的指针对象
             */
            class LuaPointer : public LuaManagedObject
            {
            private:
                LuaUserdataRef _value;
                bool _needFree;

            public:

                /**
                 * 初始化
                 * @param context 上下文对象
                 */
                LuaPointer (LuaContext *context);

                /**
                 * 初始化
                 * @param context 上下文对象
                 * @param userdata Lua自定义数据
                 */
                LuaPointer (LuaContext *context, LuaUserdataRef userdata);

                /**
                 * 初始化
                 * @param context 上下文对象
                 * @param value 任意类型指针
                 */
                LuaPointer (LuaContext *context, const void *value);

                /**
                 * 析构方法
                 */
                virtual ~LuaPointer();
                
                /**
                 * 初始化, 在反序列化对象时会触发该方法
                 *
                 * @param decoder 解码器
                 */
                LuaPointer (LuaObjectDecoder *decoder);
                
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
                 * 获取指针值
                 */
                const LuaUserdataRef getValue();

            public:

                /**
                 * 入栈数据
                 * @param context 上下文
                 */
                virtual void push(LuaContext *context);

                /**
                 * 入栈数据
                 * @param state 状态
                 * @param queue 队列
                 */
                virtual void push(lua_State *state, LuaOperationQueue *queue);
            };

        }
    }
}


#endif //ANDROID_LUAPOINTER_H
