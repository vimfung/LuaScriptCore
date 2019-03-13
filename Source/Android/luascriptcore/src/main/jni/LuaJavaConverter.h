//
// Created by 冯鸿杰 on 16/9/30.
//

#ifndef ANDROID_LUAJAVACONVERTER_H
#define ANDROID_LUAJAVACONVERTER_H

#include <jni.h>

using namespace cn::vimfung::luascriptcore;

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaValue;
            class LuaThread;
            class LuaScriptController;

        }
    }
}

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
     * 转换Java中的LuaThread为C++中的LuaThread
     *
     * @param env JNI环境
     * @param scriptController Java中的LuaScriptController对象
     * @return C++中的LuaScriptController实例对象
     */
    static LuaScriptController* convertToScriptControllerByJScriptController(JNIEnv *env, jobject scriptController);

    /**
     * 转换Java中的Object为C++中的LuaValue
     *
     * @param env JNI环境
     * @param context 上下文对象
     * @param object Java中的Object实例对象
     *
     * @return C++中的LuaValue实例对象
     */
    static LuaValue* convertToLuaValueByJObject(JNIEnv *env, LuaContext *context, jobject object);

    /**
     * 转换Java中的LuaValue为C++中的LuaValue
     *
     * @param env JNI环境
     * @param context 上下文对象
     * @param value Java中的LuaValue实例对象
     *
     * @return C++中的LuaValue实例对象
     */
    static LuaValue* convertToLuaValueByJLuaValue(JNIEnv *env, LuaContext *context, jobject value);

public:

    /**
     * 转换C++中的LuaValue为Java中的Object
     *
     * @param env JNI环境
     * @param context 上下文对象
     * @param luaValue C++中的LuaValue实例对象
     *
     * @return Java中的Object实例对象
     */
    static jobject convertToJavaObjectByLuaValue(JNIEnv *env, LuaContext *context, LuaValue *luaValue);

    /**
     * 转换C++中的LuaValue为Java中的LuaValue
     *
     * @param env JNI环境
     * @param context 上下文对象
     * @param luaValue C++中的LuaValue对象
     *
     * @return Java中的LuaValue实例对象
     */
    static jobject convertToJavaLuaValueByLuaValue(JNIEnv *env, LuaContext *context, LuaValue *luaValue);
};


#endif //ANDROID_LUAJAVACONVERTER_H
