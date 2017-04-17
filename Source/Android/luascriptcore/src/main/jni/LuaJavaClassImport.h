//
// Created by 冯鸿杰 on 17/3/27.
//

#ifndef ANDROID_LUAJAVACLASSIMPORT_H
#define ANDROID_LUAJAVACLASSIMPORT_H

#include "LuaClassImport.h"
#include <set>
#include <jni.h>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;
        }
    }
}

/**
 * Java类型导入器
 */
class LuaJavaClassImport : public cn::vimfung::luascriptcore::modules::oo::LuaClassImport
{
private:

public:
    LuaJavaClassImport();

    /**
     * 注册模块时调用
     *
     * @param name 模块名称
     * @param context 上下文对象
     */
    virtual void onRegister(const std::string &name,
                            cn::vimfung::luascriptcore::LuaContext *context);

public:

    /**
     * 设置导出类型列表
     */
    void setExportClassList(const std::list<jclass> &exportClassList);
};


#endif //ANDROID_LUAJAVACLASSIMPORT_H
