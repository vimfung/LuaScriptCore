//
// Created by 冯鸿杰 on 17/3/22.
//

#ifndef ANDROID_LUACLASSPROXY_H
#define ANDROID_LUACLASSPROXY_H

#include "LuaObject.h"
#include <list>
#include <string>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaObjectDescriptor;

            namespace modules
            {
                namespace oo
                {
                    /**
                     * 导出方法名称列表
                     */
                    typedef std::list<std::string> LuaExportNameList;

                    /**
                     * 导出类代理
                     */
                    class LuaExportClassProxy : public LuaObject
                    {
                    public:

                        /**
                         * 获取类型
                         *
                         * @return 类型对象描述器
                         */
                        virtual LuaObjectDescriptor* getExportClass();

                        /**
                         * 获取所有类方法
                         *
                         * @return 方法名称列表
                         */
                        virtual LuaExportNameList allExportClassMethods();

                        /**
                         * 获取所有实例方法
                         *
                         * @return 方法名称列表
                         */
                        virtual LuaExportNameList allExportInstanceMethods();

                        /**
                         * 获取所有读权限字段
                         *
                         * @return 字段名称列表
                         */
                        virtual LuaExportNameList allExportGetterFields();

                        /**
                         * 获取所有写权限字段
                         *
                         * @return 字段名称列表
                         */
                        virtual LuaExportNameList allExportSetterFields();
                    };

                }
            }
        }
    }
}


#endif //ANDROID_LUACLASSPROXY_H
