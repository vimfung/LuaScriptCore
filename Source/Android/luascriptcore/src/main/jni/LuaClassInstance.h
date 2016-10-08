//
// Created by 冯鸿杰 on 16/9/30.
//

#ifndef ANDROID_LUACLASSINSTANCE_H
#define ANDROID_LUACLASSINSTANCE_H

#include "LuaContext.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            namespace modules
            {
                namespace oo
                {
                    /**
                     * Lua类实例对象
                     */
                    class LuaClassInstance
                    {
                    private:
                        LuaContext *_context;
                        int _index;

                    public:

                        /**
                         * 初始化Lua类实例对象
                         *
                         * @param context 上下文对象
                         * @param index 栈中位置
                         */
                        LuaClassInstance(LuaContext *context, int index);

                    public:

                        /**
                         * 获取字段值
                         *
                         * @param name 字段名称
                         *
                         * @return 字段值
                         */
                        LuaValue* getField(std::string name);

                        /**
                         * 设置字段值
                         *
                         * @param name 字段名称
                         * @param value 字段值
                         */
                        void setField(std::string name, LuaValue *value);

                        /**
                         * 调用方法
                         *
                         * @param methodName 方法名称
                         * @param arguments 参数列表
                         *
                         * @return 返回值
                         */
                        LuaValue* callMethod(std::string methodName, LuaArgumentList *arguments);
                    };
                }
            }
        }
    }
}


#endif //ANDROID_LUACLASSINSTANCE_H
