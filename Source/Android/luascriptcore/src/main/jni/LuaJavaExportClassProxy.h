//
// Created by 冯鸿杰 on 17/3/27.
//

#ifndef ANDROID_LUAJAVAEXPORTCLASSPROXY_H
#define ANDROID_LUAJAVAEXPORTCLASSPROXY_H

#include "LuaExportClassProxy.h"
#include <string>

class LuaJavaObjectDescriptor;

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaObjectDescriptor;
        }
    }
}

/**
 * 导出Java类代理
 */
class LuaJavaExportClassProxy : public cn::vimfung::luascriptcore::modules::oo::LuaExportClassProxy
{
private:
    LuaJavaObjectDescriptor *_classDescriptor;

public:
    /**
     *  初始化
     *
     *  @param className 类型名称
     */
    LuaJavaExportClassProxy(const std::string &className);

    /**
     * 析构对象
     */
    ~LuaJavaExportClassProxy();

    /**
     * 获取类型
     *
     * @return 类型对象描述器
     */
    cn::vimfung::luascriptcore::LuaObjectDescriptor* getExportClass();

    /**
     * 获取所有类方法
     *
     * @return 方法名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportClassMethods();

    /**
     * 获取所有实例方法
     *
     * @return 方法名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportInstanceMethods();

    /**
     * 获取所有读权限字段
     *
     * @return 字段名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportGetterFields();

    /**
     * 获取所有写权限字段
     *
     * @return 字段名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportSetterFields();
};


#endif //ANDROID_LUAJAVAEXPORTCLASSPROXY_H
