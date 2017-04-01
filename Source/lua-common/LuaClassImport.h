//
// Created by 冯鸿杰 on 17/3/22.
//

#ifndef ANDROID_LUACLASSIMPORT_H
#define ANDROID_LUACLASSIMPORT_H

#include "LuaModule.h"
#include "LuaObjectDescriptor.h"
#include "LuaValue.h"
#include "LuaContext.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;
            class LuaObjectDescriptor;

            namespace modules
            {
                namespace oo
                {

                    class LuaExportClassProxy;

                    /**
                     * 是否允许导出类型处理器
                     */
                    typedef bool (*LuaAllowExportsClassHandler) (const std::string &className);

                    /**
                     * 导出类型处理器
                     */
                    typedef LuaExportClassProxy* (*LuaExportClassHandler) (const std::string &className);

                    /**
                     * 创建实例处理器
                     */
                    typedef LuaObjectDescriptor* (*LuaCreateInstanceHandler) (LuaObjectDescriptor *classDescriptor);

                    /**
                     * 类方法调用处理器
                     */
                    typedef LuaValue* (*LuaClassMethodInvokeHandler) (LuaContext *context, LuaObjectDescriptor *classDescriptor, std::string methodName, LuaArgumentList args);

                    /**
                     * 实例方法调用处理器
                     */
                    typedef LuaValue* (*LuaInstanceMethodInvokeHandler) (LuaContext *context, LuaObjectDescriptor *classDescriptor, LuaUserdataRef instance, std::string methodName, LuaArgumentList args);

                    /**
                     * 实例字段获取调用处理器
                     */
                    typedef LuaValue* (*LuaInstanceFieldGetterInvokeHandler) (LuaContext *context, LuaObjectDescriptor *classDescriptor, LuaUserdataRef instance, std::string fieldName);

                    /**
                     * 实例字段设置调用处理器
                     */
                    typedef void (*LuaInstanceFieldSetterInvokeHandler) (LuaContext *context, LuaObjectDescriptor *classDescriptor, LuaUserdataRef instance, std::string fieldName, LuaValue *value);

                    /**
                     * 原生类型导入到Lua时使用
                     */
                    class LuaClassImport : public LuaModule
                    {
                    public:
                        /**
                         * 注册模块时调用
                         *
                         * @param name 模块名称
                         * @param context 上下文对象
                         */
                        virtual void onRegister(const std::string &name,
                                                LuaContext *context);

                    public:

                        /**
                         * 为对象设置Lua元表, 如果对象的类型不在导出类型表中，则不会进行元表设置
                         *
                         * @param context 上下文对象
                         * @param className 对象的类型名称
                         * @param objectDescriptor 对象描述器
                         *
                         * @return true 设置成功， false 设置失败
                         */
                        static bool setLuaMetatable(LuaContext *context, const std::string &className, LuaObjectDescriptor *objectDescriptor);

                        /**
                         * 当导出类型时触发，用于判断是否允许导出该类型
                         *
                         * @param handler 处理器
                         */
                        static void onAllowExportsClass(LuaAllowExportsClassHandler handler);

                        /**
                         * 当类型需要导出时触发
                         *
                         * @param handler 处理器
                         */
                        static void onExportsClass(LuaExportClassHandler handler);

                        /**
                         * 当创建实例对象时触发
                         *
                         * @param handler 处理器
                         */
                        static void onCreateInstance(LuaCreateInstanceHandler handler);

                        /**
                         * 当类方法调用时触发
                         *
                         * @param handler 处理器
                         */
                        static void onClassMethodInvoke(LuaClassMethodInvokeHandler handler);

                        /**
                         * 当实例方法调用时触发
                         *
                         * @param handler 处理器
                         */
                        static void onInstanceMethodInvoke(LuaInstanceMethodInvokeHandler handler);

                        /**
                         * 当实例字段获取时触发
                         *
                         * @param handler 处理器
                         */
                        static void onInstanceFieldGetterInvoke(LuaInstanceFieldGetterInvokeHandler handler);

                        /**
                         * 当实例字段设置时触发
                         *
                         * @param handler 处理器
                         */
                        static void onInstanceFieldSetterInvoke(LuaInstanceFieldSetterInvokeHandler handler);
                    };
                }
            }
        }
    }
}


#endif //ANDROID_LUACLASSIMPORT_H
