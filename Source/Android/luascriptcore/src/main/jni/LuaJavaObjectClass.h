//
// Created by 冯鸿杰 on 16/10/8.
//

#ifndef ANDROID_LUAJAVAOBJECTCLASS_H
#define ANDROID_LUAJAVAOBJECTCLASS_H

#include "LuaObjectClass.h"
#include "LuaJavaObjectDescriptor.h"
#include <jni.h>

/**
 * Java的类型
 */
class LuaJavaObjectClass : public cn::vimfung::luascriptcore::modules::oo::LuaObjectClass
{
private:
    jclass  _moduleClass;
    jobjectArray _fields;
    jobjectArray _instanceMethods;
    jobjectArray _classMethods;

public:

    /**
     * 初始化Java类型
     *
     * @param env JNI环境
     * @param superClass 父级类型
     * @param moduleClass 类型
     * @param fields 导出字段集合
     * @param instanceMethods 导出实例方法集合
     * @param classMethods 导出类方法集合
     */
    LuaJavaObjectClass(JNIEnv *env,
                       LuaJavaObjectClass *superClass,
                       jclass moduleClass,
                       jobjectArray fields,
                       jobjectArray instanceMethods,
                       jobjectArray classMethods);
public:

    /**
     * 获取模块类型
     *
     * @param env JNI环境
     *
     * @return 模块类型
     */
    jclass getModuleClass(JNIEnv *env);

public:


    /**
     * 判断是否为指定类型的子类
     *
     * @param type 类型
     *
     * @return true 是 false 不是
     */
    bool subclassOf(cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *type);

    /**
     * 注册模块时调用
     *
     * @param name 模块名称
     * @param context 上下文对象
     */
    void onRegister(const std::string &name,
                    cn::vimfung::luascriptcore::LuaContext *context);

    /**
     * 创建Lua实例对象
     *
     * @param objectDescriptor 对象描述器
     */
    void createLuaInstance(cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor *objectDescriptor);
};


#endif //ANDROID_LUAJAVAOBJECTCLASS_H
