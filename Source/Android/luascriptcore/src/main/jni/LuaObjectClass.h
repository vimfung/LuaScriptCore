//
// Created by 冯鸿杰 on 16/9/27.
//

#ifndef ANDROID_LUAOBJECTCLASS_H
#define ANDROID_LUAOBJECTCLASS_H

#include "LuaModule.h"
#include "LuaClassInstance.h"

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
                    class LuaClassInstance;
                    class LuaObjectClass;

                    /**
                     * 类对象实例化处理器
                     *
                     * @param instance 类实例
                     */
                    typedef void (*LuaClassObjectCreatedHandler) (LuaClassInstance *instance);

                    /**
                     * 类对象实例销毁处理器
                     *
                     * @param instance 类实例
                     */
                    typedef void (*LuaClassObjectDestroyHandler) (LuaClassInstance *instance);

                    /**
                     * 类对象实例获取描述处理器
                     *
                     * @param instance 类实例
                     *
                     * @return 对象描述
                     */
                    typedef std::string (*LuaClassObjectGetDescriptionHandler) (LuaClassInstance *instance);

                    /**
                     * 子类化事件处理器
                     *
                     * @param objectClass 对象类型
                     */
                    typedef void (*LuaSubClassHandler) (LuaObjectClass *objectClass);

                    /**
                     * Lua类描述对象
                     */
                    class LuaObjectClass : public LuaModule
                    {
                    private:
                        /**
                         * 父类名称
                         */
                        std::string _superClassName;

                        /**
                         * 类对象实例化处理器
                         */
                        LuaClassObjectCreatedHandler _classObjectCreatedHandler;

                        /**
                         * 类对象实例销毁处理器
                         */
                        LuaClassObjectDestroyHandler _classObjectDestroyHandler;

                        /**
                         * 类对象实例描述处理器
                         */
                        LuaClassObjectGetDescriptionHandler _classObjectDescriptionHandler;

                        /**
                         * 子类化处理器
                         */
                        LuaSubClassHandler  _subclassHandler;

                    public:

                        /**
                         * 初始化Lua类描述对象
                         *
                         * @param superClassName 父级类型
                         */
                        LuaObjectClass(const std::string &superClassName);

                    public:

                        /**
                         * 对象创建时触发
                         *
                         * @param handler 事件处理器
                         */
                        void onObjectCreated(LuaClassObjectCreatedHandler handler);

                        /**
                         * 对象销毁时触发
                         *
                         * @param handler 事件处理器
                         */
                        void onObjectDestroy(LuaClassObjectDestroyHandler handler);

                        /**
                         * 对象获取描述时触发
                         *
                         * @param handler 事件处理器
                         */
                        void onObjectGetDescription (LuaClassObjectGetDescriptionHandler handler);

                        /**
                         * 子类化时触发
                         *
                         * @param handler 事件处理器
                         */
                        void onSubClass (LuaSubClassHandler handler);

                        /**
                         * 获取对象生成事件处理器
                         *
                         * @return 事件处理器
                         */
                        LuaClassObjectCreatedHandler getObjectCreatedHandler();

                        /**
                         * 获取对象销毁事件处理器
                         *
                         * @return 事件处理器
                         */
                        LuaClassObjectDestroyHandler getObjectDestroyHandler();

                        /**
                         * 获取对象描述事件处理器
                         *
                         * @return 事件处理器
                         */
                        LuaClassObjectGetDescriptionHandler getObjectDescriptionHandler();

                        /**
                         * 获取子类化事件处理器
                         *
                         * @return 事件处理器
                         */
                        LuaSubClassHandler getSubClassHandler();

                    public:

                        /**
                         * 注册模块时调用
                         *
                         * @param name 模块名称
                         * @param context 上下文对象
                         */
                        virtual void onRegister(const std::string &name,
                                                cn::vimfung::luascriptcore::LuaContext *context);
                    };
                }
            }
        }
    }
}




#endif //ANDROID_LUAOBJECTCLASS_H
