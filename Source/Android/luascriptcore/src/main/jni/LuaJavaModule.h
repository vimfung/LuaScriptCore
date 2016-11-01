//
// Created by 冯鸿杰 on 16/9/28.
//

#ifndef ANDROID_LUAJAVAMODULE_H
#define ANDROID_LUAJAVAMODULE_H

#include <jni.h>
#include "LuaModule.h"

/**
 * Java层的模块
 */
class LuaJavaModule : public cn::vimfung::luascriptcore::LuaModule
{
private:
    JNIEnv *_env;
    jclass  _moduleClass;
    jobjectArray _methods;

public:

    /**
     * 创建Java模块
     *
     * @param env JNI环境
     * @param moduleClass Java层的模块类型
     * @param methods 导出方法集合
     */
    LuaJavaModule(
            JNIEnv *env,
            jclass moduleClass,
            jobjectArray methods);

public:

    /**
     * 获取模块类型
     *
     * @return 模块类型
     */
    jclass getModuleClass();

public:

    /**
     * 注册模块时调用, 重写LuaModule中实现
     *
     * @param name 模块名称
     * @param context 上下文对象
     */
    void onRegister(const std::string &name, cn::vimfung::luascriptcore::LuaContext *context);
};


#endif //ANDROID_LUAJAVAMODULE_H
