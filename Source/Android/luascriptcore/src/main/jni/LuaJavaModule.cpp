//
// Created by 冯鸿杰 on 16/9/28.
//

#include <stdint.h>
#include "LuaJavaModule.h"
#include "LuaJavaType.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaJavaConverter.h"

/**
 * Lua模块方法处理器
 *
 * @param module 模块对象
 * @param methodName 方法名称
 * @param arguments 方法参数列表
 *
 * @return 方法返回值
 */
static LuaValue* _luaModuleMethodHandler (LuaModule *module, std::string methodName, LuaArgumentList arguments)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    LuaJavaModule *jmodule = (LuaJavaModule *)module;
    if (jmodule != NULL)
    {
        static jclass moduleClass = jmodule -> getModuleClass(env);
        static jmethodID invokeMethodID = env -> GetStaticMethodID(LuaJavaType::moduleClass(env), "_methodInvoke", "(Ljava/lang/Class;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");

        static jclass luaValueClass = LuaJavaType::luaValueClass(env);

        jstring jMethodName = env -> NewStringUTF(methodName.c_str());

        //参数
        jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
        int index = 0;
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
        {
            LuaValue *argument = *it;
            jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, jmodule -> getContext(), argument);
            env -> SetObjectArrayElement(argumentArr, index, jArgument);
            index++;
        }

        jobject result = env -> CallStaticObjectMethod(moduleClass, invokeMethodID, moduleClass, jMethodName, argumentArr);
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, jmodule -> getContext(), result);
    }

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

LuaJavaModule::LuaJavaModule(
        JNIEnv *env,
        jclass moduleClass,
        jobjectArray methods)
{
    _moduleClass = (jclass)env -> NewWeakGlobalRef(moduleClass);
    _methods = methods;
}

jclass LuaJavaModule::getModuleClass(JNIEnv *env)
{
    jclass retClass = NULL;

    if (env -> IsSameObject(_moduleClass, NULL) != JNI_TRUE)
    {
        retClass = _moduleClass;
    }

    return retClass;
}

void LuaJavaModule::onRegister(const std::string &name, cn::vimfung::luascriptcore::LuaContext *context)
{
    cn::vimfung::luascriptcore::LuaModule::onRegister(name, context);

    JNIEnv *env = LuaJavaEnv::getEnv();

    //注册模块方法
    jsize methodCount = env -> GetArrayLength(_methods);
    jclass jmethodClass = LuaJavaType::methodClass(env);
    jmethodID getMethodNameMethodId = env -> GetMethodID(jmethodClass, "getName", "()Ljava/lang/String;");
    for (int i = 0; i < methodCount; ++i)
    {
        jobject method = env -> GetObjectArrayElement(_methods, i);
        jstring methodName = (jstring)env -> CallObjectMethod(method, getMethodNameMethodId);

        const char *methodNameCStr = env -> GetStringUTFChars(methodName, NULL);
        this -> registerMethod(methodNameCStr, _luaModuleMethodHandler);
        env -> ReleaseStringUTFChars(methodName, methodNameCStr);
    }

    LuaJavaEnv::resetEnv(env);
}
