//
// Created by 冯鸿杰 on 2017/12/1.
//

#ifndef ANDROID_LUAJAVAEXPORTPROPERTYDESCRIPTOR_H
#define ANDROID_LUAJAVAEXPORTPROPERTYDESCRIPTOR_H

#include "LuaExportPropertyDescriptor.hpp"

using namespace cn::vimfung::luascriptcore;

/**
 * Java导出属性描述
 */
class LuaJavaExportPropertyDescriptor : public LuaExportPropertyDescriptor
{
public:

    /**
     * 初始化
     *
     * @param name 属性名称
     * @param canRead 是否可读
     * @param canWrite 是否可写
     */
    LuaJavaExportPropertyDescriptor(std::string name,
                                    bool canRead,
                                    bool canWrite);

    /**
     调用Getter方法

     @param session 会话
     @param instance 实例对象
     @return 返回值
     */
    virtual LuaValue* invokeGetter(LuaSession *session, LuaObjectDescriptor *instance);


    /**
     调用Setter方法

     @param session 会话
     @param instance 实例对象
     @param value 属性值
     */
    virtual void invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value);
};


#endif //ANDROID_LUAJAVAEXPORTPROPERTYDESCRIPTOR_H
