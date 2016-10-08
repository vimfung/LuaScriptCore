//
// Created by 冯鸿杰 on 16/9/30.
//

#ifndef ANDROID_LUAJAVACONVERTER_H
#define ANDROID_LUAJAVACONVERTER_H

#include "LuaContext.h"
#include <jni.h>

using namespace cn::vimfung::luascriptcore;

/**
 * Java转换器
 */
class LuaJavaConverter
{
public:

    /**
     * 转换Java中的LuaContext为C++中的LuaContext
     *
     * @param env JNI环境
     * @param context Java中的LuaContext实例对象
     *
     * @return C++中的LuaContext实例对象
     */
    static LuaContext* convertToContextByJLuaContext(JNIEnv *env, jobject context);

    /**
     * 转换Java中的Object为C++中的LuaValue
     *
     * @param env JNI环境
     * @param object Java中的Object实例对象
     *
     * @return C++中的LuaValue实例对象
     */
    static LuaValue* convertToLuaValueByJObject(JNIEnv *env, jobject object);

    /**
     * 转换Java中的LuaValue为C++中的LuaValue
     *
     * @param env JNI环境
     * @param value Java中的LuaValue实例对象
     *
     * @return C++中的LuaValue实例对象
     */
    static LuaValue* convertToLuaValueByJLuaValue(JNIEnv *env, jobject value);

public:

    /**
     * 转换C++中的LuaValue为Java中的Object
     *
     * @param env JNI环境
     * @param luaValue C++中的LuaValue实例对象
     *
     * @return Java中的Object实例对象
     */
    static jobject convertToJavaObjectByLuaValue(JNIEnv *env, LuaValue *luaValue);

    /**
     * 转换C++中的LuaValue为Java中的LuaValue
     *
     * @param env JNI环境
     * @param luaValue C++中的LuaValue对象
     *
     * @return Java中的LuaValue实例对象
     */
    static jobject convertToJavaLuaValueByLuaValue(JNIEnv *env, LuaValue *luaValue);
};


#endif //ANDROID_LUAJAVACONVERTER_H
