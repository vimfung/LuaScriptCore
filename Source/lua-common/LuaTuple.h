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

            /**
             * 元组
             */
            class LuaTuple : public LuaObject
            {
            private:
                LuaValueList _returnValues;

            public:
                LuaTuple ();
                ~LuaTuple();

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
                LuaValue *getResturValueByIndex(int index);

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
