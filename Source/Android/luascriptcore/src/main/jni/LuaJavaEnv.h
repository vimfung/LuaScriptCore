//
// Created by 冯鸿杰 on 16/9/29.
//

#ifndef ANDROID_LUAJAVAENV_H
#define ANDROID_LUAJAVAENV_H

#include <jni.h>
#include "LuaContext.h"
#include "LuaJavaModule.h"

using namespace cn::vimfung::luascriptcore;

class LuaJavaModule;

/**
 * JNI的环境对象
 */
class LuaJavaEnv
{
public:

    /**
     * 初始化环境
     *
     * @param javaVM Java虚拟机对象
     */
    static void init(JavaVM *javaVM);

    /**
     * 获取JNI环境
     *
     * @return JNI环境
     */
    static JNIEnv* getEnv();

    /**
     * 重置JNI环境状态,由于通过getEnv方法获取环境时,可能导致env需要附加到线程上,因此在调用getEnv后,在不使用env时调用此方法进行重置。
     *
     * @param env JNI环境
     */
    static void resetEnv(JNIEnv *env);

    /**
     * 创建Java的上下文对象
     *
     * @param env JNI环境
     * @param context 上下文对象
     *
     * @return Java中的LuaContext对象
     */
    static jobject createJavaLuaContext(JNIEnv *env, LuaContext *context);

    /**
     * 获取Java中的LuaContext对象
     *
     * @param env JNI环境
     * @param context 上下文对象
     *
     * @return Java中的LuaContext对象
     */
    static jobject getJavaLuaContext(JNIEnv *env, LuaContext *context);

    /**
     * 创建Java中的LuaModule对象
     *
     * @param env JNI环境
     * @param moduleClass Java的模块类型
     * @param module 模块对象
     *
     * @return Java中的LuaModule对象
     */
    static jobject createJavaLuaModule(JNIEnv *env, jclass moduleClass, LuaModule *module);

    /**
     * 获取Java中的LuaModule对象
     *
     * @param env JNI环境
     * @param module 模块对象
     *
     * @return Java中的LuaModule对象
     */
    static jobject getJavaLuaModule(JNIEnv *env, LuaModule *module);

    /**
     * 释放对象,由于Java层中对象需要引用本地对象,因此为确保Java对象释放时也释放本地对象,则需要调用该方法。
     *
     * @param env JNI环境
     * @param objectId 本地对象标识
     */
    static jobject releaseObject(JNIEnv *env, jint objectId);

    /**
     * 获取方法处理器
     *
     * @return 方法处理器
     */
    static LuaMethodHandler luaMethodHandler();

    /**
     * 获取模块方法处理器
     *
     * @return 模块方法处理器
     */
    static LuaModuleMethodHandler luaModuleMethodHandler();

    /**
     * 获取模块属性设置处理器
     *
     * @return 模块属性设置处理器
     */
    static LuaModuleSetterHandler luaModuleSetterHandler();

    /**
     * 获取模块属性获取处理器
     *
     * @return 模块属性获取处理器
     */
    static LuaModuleGetterHandler luaModuleGetterHandler();


};


#endif //ANDROID_LUAJAVAENV_H
