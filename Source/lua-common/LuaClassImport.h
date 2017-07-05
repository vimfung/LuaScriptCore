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
                    class LuaClassImport;

                    /**
                     * 检测是否为LuaObjectClass的子类，如果是则进行注册。
                     */
                    typedef bool (*LuaCheckObjectSubclassHandler) (
                            LuaContext *context,
                            LuaClassImport *classImport,
                            const std::string &className);

                    /**
                     * 是否允许导出类型处理器
                     */
                    typedef bool (*LuaAllowExportsClassHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        const std::string &className);

                    /**
                     * 导出类型处理器
                     */
                    typedef LuaExportClassProxy* (*LuaExportClassHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        const std::string &className);

                    /**
                     * 创建实例处理器
                     */
                    typedef LuaObjectDescriptor* (*LuaCreateInstanceHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        LuaObjectDescriptor *classDescriptor);

                    /**
                     * 类方法调用处理器
                     */
                    typedef LuaValue* (*LuaClassMethodInvokeHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        LuaObjectDescriptor *classDescriptor,
                        std::string methodName,
                        LuaArgumentList args);

                    /**
                     * 实例方法调用处理器
                     */
                    typedef LuaValue* (*LuaInstanceMethodInvokeHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        LuaObjectDescriptor *classDescriptor,
                        LuaObjectDescriptor *instance,
                        std::string methodName,
                        LuaArgumentList args);

                    /**
                     * 实例字段获取调用处理器
                     */
                    typedef LuaValue* (*LuaInstanceFieldGetterInvokeHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        LuaObjectDescriptor *classDescriptor,
                        LuaObjectDescriptor *instance,
                        std::string fieldName);

                    /**
                     * 实例字段设置调用处理器
                     */
                    typedef void (*LuaInstanceFieldSetterInvokeHandler) (
                        LuaContext *context,
                        LuaClassImport *classImport,
                        LuaObjectDescriptor *classDescriptor,
                        LuaObjectDescriptor *instance,
                        std::string fieldName,
                        LuaValue *value);

                    /**
                     * 原生类型导入到Lua时使用
                     */
                    class LuaClassImport : public LuaModule
                    {
                    private:
                        /**
                         * 上下文对象
                         */
                        LuaContext *_context = NULL;

                        /**
                         * 检测是否为对象子类处理器
                         */
                        LuaCheckObjectSubclassHandler _checkObjectSubclassHandler = NULL;
                        
                        /**
                         * 允许导出类型处理器
                         */
                        LuaAllowExportsClassHandler _allowExcportsClassHandler = NULL;
                        
                        /**
                         * 导出类型处理器
                         */
                        LuaExportClassHandler _exportClassHandler = NULL;
                        
                        /**
                         * 创建实例处理器
                         */
                        LuaCreateInstanceHandler _createInstanceHandler = NULL;
                        
                        /**
                         * 类方法调用处理器
                         */
                        LuaClassMethodInvokeHandler _classMethodInvokeHandler = NULL;
                        
                        /**
                         * 实例方法调用处理器
                         */
                        LuaInstanceMethodInvokeHandler _instanceMethodInvokeHandler = NULL;
                        
                        /**
                         * 实例字段获取器调用处理器
                         */
                        LuaInstanceFieldGetterInvokeHandler _instanceFieldGetterInvokeHandler = NULL;
                        
                        /**
                         * 实例字段设置器调用处理器
                         */
                        LuaInstanceFieldSetterInvokeHandler _instanceFieldSetterInvokeHandler = NULL;
                        
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
                         * 当检测导入类是否为LuaObjectClass子类是触发
                         *
                         * @param handler 处理器
                         */
                        void onCheckObjectSubclass(LuaCheckObjectSubclassHandler handler);

                        /**
                         * 当导出类型时触发，用于判断是否允许导出该类型
                         *
                         * @param handler 处理器
                         */
                        void onAllowExportsClass(LuaAllowExportsClassHandler handler);

                        /**
                         * 当类型需要导出时触发
                         *
                         * @param handler 处理器
                         */
                        void onExportsClass(LuaExportClassHandler handler);

                        /**
                         * 当创建实例对象时触发
                         *
                         * @param handler 处理器
                         */
                        void onCreateInstance(LuaCreateInstanceHandler handler);

                        /**
                         * 当类方法调用时触发
                         *
                         * @param handler 处理器
                         */
                        void onClassMethodInvoke(LuaClassMethodInvokeHandler handler);

                        /**
                         * 当实例方法调用时触发
                         *
                         * @param handler 处理器
                         */
                        void onInstanceMethodInvoke(LuaInstanceMethodInvokeHandler handler);

                        /**
                         * 当实例字段获取时触发
                         *
                         * @param handler 处理器
                         */
                        void onInstanceFieldGetterInvoke(LuaInstanceFieldGetterInvokeHandler handler);

                        /**
                         * 当实例字段设置时触发
                         *
                         * @param handler 处理器
                         */
                        void onInstanceFieldSetterInvoke(LuaInstanceFieldSetterInvokeHandler handler);
                        
                    public:
                        
                        /**
                         获取类方法调用处理器

                         @return 处理器
                         */
                        LuaClassMethodInvokeHandler getClassMethodInvokeHandler();
                        
                        /**
                         获取实例方法调用处理器

                         @return 处理器
                         */
                        LuaInstanceMethodInvokeHandler getInstanceMethodInvokeHandler();
                        
                        /**
                         获取实例字段Getter调用处理器

                         @return 处理器
                         */
                        LuaInstanceFieldGetterInvokeHandler getInstanceFieldGetterInvokeHandler();
                        
                        /**
                         获取实例字段Setter调用处理器

                         @return 处理器
                         */
                        LuaInstanceFieldSetterInvokeHandler getInstanceFieldSetterInvokeHandler();
                        
                        /**
                         获取创建实例处理器

                         @return 处理器
                         */
                        LuaCreateInstanceHandler getCreateInstanceHandler();

                        /**
                         * 获取检测对象子类处理器
                         *
                         * @return 处理器
                         */
                        LuaCheckObjectSubclassHandler getCheckObjectSubclassHandler();
                        
                        /**
                         获取是否允许导出类型处理器

                         @return 处理器
                         */
                        LuaAllowExportsClassHandler getAllowExportsClassHandler();
                        
                        /**
                         获取导出类型处理器

                         @return 处理器
                         */
                        LuaExportClassHandler getExportClassHandler();
                    };
                }
            }
        }
    }
}


#endif //ANDROID_LUACLASSIMPORT_H
