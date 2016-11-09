//
// Created by 冯鸿杰 on 16/9/27.
//

#ifndef ANDROID_LUAOBJECTCLASS_H
#define ANDROID_LUAOBJECTCLASS_H

#include "LuaModule.h"

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
                    class LuaObjectClass;

                    /**
                     * 类对象实例化处理器
                     *
                     * @param objectClass 要实例化的类型
                     */
                    typedef void (*LuaClassObjectCreatedHandler) (LuaObjectClass *objectClass);

                    /**
                     * 类对象实例销毁处理器
                     *
                     * @param instance 类实例
                     */
                    typedef void (*LuaClassObjectDestroyHandler) (LuaObjectClass *instance);

                    /**
                     * 类对象实例获取描述处理器
                     *
                     * @param instance 类实例
                     *
                     * @return 对象描述
                     */
                    typedef std::string (*LuaClassObjectGetDescriptionHandler) (LuaObjectClass *objectClass);

                    /**
                     * 子类化事件处理器
                     *
                     * @param objectClass 对象类型
                     */
                    typedef void (*LuaSubClassHandler) (LuaObjectClass *objectClass, std::string subclassName);

                    /**
                     * 类实例方法处理器
                     */
                    typedef LuaValue* (*LuaInstanceMethodHandler) (LuaObjectClass *objectClass, std::string methodName, LuaArgumentList arguments);

                    /**
                     * 类属性Getter处理器
                     */
                    typedef LuaValue* (*LuaInstanceGetterHandler) (LuaObjectClass *objectClass, std::string fieldName);

                    /**
                     * 类属性Setter处理器
                     */
                    typedef void (*LuaInstanceSetterHandler) (LuaObjectClass *objectClass, std::string fieldName, LuaValue *value);

                    /**
                     * 类方法映射表类型
                     */
                    typedef std::map<std::string, LuaInstanceMethodHandler> LuaInstanceMethodMap;

                    /**
                     *  类属性Getter映射表类型
                     */
                    typedef std::map<std::string, LuaInstanceSetterHandler> LuaInstanceSetterMap;

                    /**
                     * 类属性Setter映射表类型
                     */
                    typedef std::map<std::string, LuaInstanceGetterHandler> LuaInstanceGetterMap;

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

                        /**
                         * 实例方法表
                         */
                        LuaInstanceMethodMap _instanceMethodMap;

                        /**
                         * 实例属性Setter表
                         */
                        LuaInstanceSetterMap _instanceSetterMap;

                        /**
                         * 实例属性Getter表
                         */
                        LuaInstanceGetterMap _instanceGetterMap;

                    protected:

                        /**
                         * 是否为内部调用，如果该标识为true，部分操作行为可能改变，如__newindex处理中
                         * 如果处于内部调用则不自动添加实例方法或属性。
                         */
                        bool _isInternalCall;

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
                         * 获取是否为内部调用
                          *
                         * @return true 为内部调用，false 非内部调用
                          */
                        bool getIsInternalCall();

                        /**
                         * 注册模块时调用
                         *
                         * @param name 模块名称
                         * @param context 上下文对象
                         */
                        virtual void onRegister(const std::string &name,
                                                cn::vimfung::luascriptcore::LuaContext *context);

                        /**
                         * 创建Lua实例对象
                         *
                         * @param objectDescriptor 对象描述器
                         */
                        virtual void createLuaInstance(cn::vimfung::luascriptcore::LuaObjectDescriptor *objectDescriptor);

                        /**
                         * 入栈数据
                         *
                         * @param objectDescriptor 对象描述器
                         */
                        void push(cn::vimfung::luascriptcore::LuaObjectDescriptor *objectDescriptor);

                        /**
                         * 获取模块方法处理器, 提供Lua回调方法调用
                         *
                         * @param methodName 方法名称
                         *
                         * @return 模块方法处理器
                         */
                        LuaInstanceMethodHandler getInstanceMethodHandler(std::string methodName);

                        /**
                         * 获取属性设置处理器, 提供Lua回调方法调用
                         *
                         * @param fieldName 字段名称
                         *
                         * @return Setter方法处理器
                         */
                        LuaInstanceSetterHandler getInstanceSetterHandler(std::string fieldName);

                        /**
                         * 获取属性获取处理器, 提供Lua回调方法调用
                         *
                         * @param fieldName 字段名称
                         *
                         * @return Getter方法处理器
                         */
                        LuaInstanceGetterHandler getGetterHandler(std::string fieldName);

                    public:

                        /**
                         * 注册方法
                         *
                         * @param methodName 方法名称
                         * @param handler 方法处理器
                         */
                        void registerMethod(std::string methodName, LuaModuleMethodHandler handler);

                        /**
                         * 注册实例属性
                         *
                         * @param fieldName 字段名称
                         * @param getterHandler 获取处理器
                         * @param setterHandler 设置处理器
                         */
                        void registerInstanceField(std::string fieldName, LuaInstanceGetterHandler getterHandler, LuaInstanceSetterHandler setterHandler);

                        /**
                         * 注册实例方法
                         *
                         * @param methodName 方法名称
                         * @param handler 方法处理器
                         */
                        void registerInstanceMethod(std::string methodName, LuaInstanceMethodHandler handler);
                    };
                }
            }
        }
    }
}




#endif //ANDROID_LUAOBJECTCLASS_H
