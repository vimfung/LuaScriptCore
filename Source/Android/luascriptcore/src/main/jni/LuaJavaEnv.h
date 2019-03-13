//
// Created by 冯鸿杰 on 16/9/29.
//

#ifndef ANDROID_LUAJAVAENV_H
#define ANDROID_LUAJAVAENV_H

#include <jni.h>
#include <LuaValue.h>
#include "LuaDefined.h"

using namespace cn::vimfung::luascriptcore;

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaObjectDescriptor;

        }
    }
}

/**
 * JNI的环境对象
 */
class LuaJavaEnv : public LuaObject
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
     * @param config 上下文配置
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
     * 创建Java的脚本控制器对象
     * @param env
     * @param scriptController
     * @return
     */
    static jobject createJavaLuaScriptController(JNIEnv *env, LuaScriptController *scriptController);

    /**
     * 关联对象实例
     *
     * @param env JNI环境
     * @param instance Java中的实例对象
     * @param descriptor 对象描述
     */
    static void associcateInstance(JNIEnv *env, jobject instance, LuaObjectDescriptor *descriptor);

    /**
     * 移除关联对象实例
     *
     * @param env JNI环境
     * @param instance Java中的实例对象
     */
    static void removeAssociateInstance(JNIEnv *env, jobject instance);

    /**
     * 获取关联对象实例引用
     *
     * @param env JNI环境
     * @param instance Java中的实例对象
     *
     * @return 关联实例对象的引用
     */
    static LuaObjectDescriptor* getAssociateInstanceRef(JNIEnv *env, jobject instance);

    /**
     * 释放对象,由于Java层中对象需要引用本地对象,因此为确保Java对象释放时也释放本地对象,则需要调用该方法。
     *
     * @param env JNI环境
     * @param objectId 本地对象标识
     */
    static void releaseObject(JNIEnv *env, jint objectId);

    /**
     * 获取方法处理器
     *
     * @return 方法处理器
     */
    static LuaMethodHandler luaMethodHandler();

    /**
     * 根据实例获取类名
     *
     * @param env JNI环境
     * @param instance 实例对象
     *
     * @return 类名
     */
    static std::string getJavaClassNameByInstance(JNIEnv *env, jobject instance);

    /**
     * 获取异常处理方法
     *
     * @return Lua异常处理器
     */
    static LuaExceptionHandler getExceptionHandler();

    /**
     * 获取导出原生类型处理器
     * @return 导出原生类型处理器
     */
    static LuaExportsNativeTypeHandler getExportsNativeTypeHandler();

    /**
     * 获取Java类型的名称
     *
     * @param env JNI环境
     * @param cls 类型
     * @param simpleName 是否返回简单的类名，true 表示返回类名不带包名，false 表示带包名
     *
     * @return 类型名称
     */
    static std::string getJavaClassName(JNIEnv *env, jclass cls, bool simpleName);

    /**
     * 获取导出类型管理器
     *
     * @param env JNI环境
     *
     * @return 导出类型管理器
     */
    static jobject getExportTypeManager(JNIEnv *env);

    /**
     * 查找类型
     *
     * @param env JNI环境
     * @param className 类型名称
     * @return 类型
     */
    static jclass findClass(JNIEnv *env, std::string className);

private:

    JNIEnv *_jniEnv;
    bool _attachedThread;
    int _count;

};


#endif //ANDROID_LUAJAVAENV_H
