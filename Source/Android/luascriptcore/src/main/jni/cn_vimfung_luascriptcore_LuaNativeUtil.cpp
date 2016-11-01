//
// Created by vimfung on 16/8/29.
//
#include <stddef.h>
#include <ctype.h>
#include "cn_vimfung_luascriptcore_LuaNativeUtil.h"

#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaModule.h"
#include "LuaJavaModule.h"
#include "LuaJavaConverter.h"
#include "LuaObjectManager.h"
#include "LuaDefine.h"
#include "LuaJavaEnv.h"
#include "LuaJavaType.h"
#include "LuaObjectClass.h"
#include "LuaJavaObjectClass.h"

using namespace cn::vimfung::luascriptcore;
using namespace cn::vimfung::luascriptcore::modules::oo;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LuaJavaEnv::init(vm);
    return JNI_VERSION_1_4;
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    createContext
 * Signature: ()Lcn/vimfung/luascriptcore/LuaContext;
 */
JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_createContext
        (JNIEnv *env, jclass obj)
{
    LuaContext *context = new LuaContext();
    jobject jcontext = LuaJavaEnv::createJavaLuaContext(env, context);
    context -> release();

    return jcontext;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_addSearchPath
        (JNIEnv *env, jclass thiz, jint nativeContextId, jstring path)
{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char *pathStr = env -> GetStringUTFChars(path, NULL);
        context -> addSearchPath(pathStr);
        env -> ReleaseStringUTFChars(path, pathStr);
    }
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    evalScript
 * Signature: (ILjava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;
 */
JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_evalScript
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring script)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char* scriptText = env ->GetStringUTFChars(script, NULL);
        LuaValue *value = context->evalScript(scriptText);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
        value -> release();
        env -> ReleaseStringUTFChars(script, scriptText);
    }

    return retObj;

}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_evalScriptFromFile
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring path)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char* scriptPath = env ->GetStringUTFChars(path, NULL);

        LuaValue *value = context->evalScriptFromFile(scriptPath);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
        value -> release();
        env -> ReleaseStringUTFChars(path, scriptPath);
    }

    return retObj;
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_callMethod
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring methodName, jobjectArray arguments)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        LuaArgumentList argumentList;

        if (arguments != NULL) {
            jsize length = env->GetArrayLength(arguments);
            for (int i = 0; i < length; ++i) {
                jobject item = env->GetObjectArrayElement(arguments, i);
                LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, item);
                if (value != NULL) {
                    argumentList.push_back(value);
                }
            }
        }

        const char *methodNameStr = env -> GetStringUTFChars(methodName, NULL);
        LuaValue *retValue = context->callMethod(methodNameStr, &argumentList);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, retValue);
        retValue -> release();
        env -> ReleaseStringUTFChars(methodName, methodNameStr);

        //释放参数内存
        for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    return retObj;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_registerMethod
        (JNIEnv *env, jclass thiz, jint nativeContextId, jstring methodName)
{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char *methodNameStr = env -> GetStringUTFChars(methodName, NULL);
        context -> registerMethod(methodNameStr, LuaJavaEnv::luaMethodHandler());
        env -> ReleaseStringUTFChars(methodName, methodNameStr);
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_releaseNativeObject
(JNIEnv *env, jclass obj, jint nativeObjectId)
{
    LuaJavaEnv::releaseObject(env, nativeObjectId);
}

JNIEXPORT jboolean JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_registerModule
        (JNIEnv *env, jclass thiz, jint nativeContextId, jstring moduleName, jclass moduleClass, jobjectArray methods)

{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        //实例化Module
        const char *moduleNameCStr = env -> GetStringUTFChars(moduleName, NULL);

        //创建Java模块
        LuaJavaModule *javaModule = new LuaJavaModule(
                env,
                moduleClass,
                methods);
        context -> registerModule(moduleNameCStr, javaModule);

        javaModule -> release();

        env -> ReleaseStringUTFChars(moduleName, moduleNameCStr);

        return JNI_TRUE;
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_isModuleRegisted
        (JNIEnv *env, jclass thiz, jint nativeContextId, jstring moduleName)
{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char *modNameCStr = env -> GetStringUTFChars(moduleName, NULL);
        bool registed = context -> isModuleRegisted(modNameCStr);
        env -> ReleaseStringUTFChars(moduleName, modNameCStr);

        return (jboolean)registed;
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_registerClass
        (JNIEnv *env, jclass thiz, jobject jcontext, jstring className, jstring superClassName, jclass jobjectClass, jobjectArray fields, jobjectArray instanceMethods, jobjectArray classMethods)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        const char *classNameStr = env -> GetStringUTFChars(className, NULL);

        std::string superClassNameStr;
        const char *superClassNameCStr = NULL;

        if (superClassName != NULL)
        {
            superClassNameCStr = env -> GetStringUTFChars(superClassName, NULL);
            superClassNameStr = superClassNameCStr;
        }

        LuaJavaObjectClass *objectClass = new LuaJavaObjectClass(
                env,
                (const std::string)superClassNameStr,
                jobjectClass,
                fields,
                instanceMethods,
                classMethods);
        context -> registerModule(classNameStr, objectClass);

        objectClass -> release();

        if (superClassNameCStr != NULL)
        {
            env -> ReleaseStringUTFChars(superClassName, superClassNameCStr);
        }
        env -> ReleaseStringUTFChars(className, classNameStr);

        return JNI_TRUE;
    }

    return JNI_FALSE;
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_invokeFunction
        (JNIEnv *env, jclass thiz, jobject jcontext, jobject func, jobjectArray arguments)
{
    jobject retObj = NULL;
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        //获取LuaFunction
        LuaValue *value = LuaJavaConverter::convertToLuaValueByJObject(env, func);
        if (value != NULL)
        {
            LuaArgumentList argumentList;

            if (arguments != NULL) {
                jsize length = env->GetArrayLength(arguments);
                for (int i = 0; i < length; ++i) {
                    jobject item = env->GetObjectArrayElement(arguments, i);
                    LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, item);
                    if (value != NULL) {
                        argumentList.push_back(value);
                    }
                }
            }

            LuaValue *retValue = value -> toFunction() -> invoke(&argumentList);
            retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, retValue);
            retValue -> release();

            //释放参数内存
            for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; ++it)
            {
                LuaValue *item = *it;
                item -> release();
            }

            value -> release();
        }
    }

    return retObj;
}