//
// Created by vimfung on 16/8/29.
//
#include <stddef.h>
#include <ctype.h>
#include "cn_vimfung_luascriptcore_LuaNativeUtil.h"

#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaJavaConverter.h"
#include "LuaObjectManager.h"
#include "LuaDefine.h"
#include "LuaJavaEnv.h"
#include "LuaJavaType.h"
#include "LuaFunction.h"
#include "LuaJavaExportTypeDescriptor.h"
#include "LuaJavaExportMethodDescriptor.h"
#include "LuaJavaExportPropertyDescriptor.h"
#include "LuaExportsTypeManager.hpp"
#include "StringUtils.h"
#include "LuaCoroutine.h"
#include "LuaScriptController.h"

using namespace cn::vimfung::luascriptcore;

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
    LuaContext *context = new LuaContext("android");
    jobject jcontext = LuaJavaEnv::createJavaLuaContext(env, context);
    context -> release();

    //绑定导出原生类型方法
    context->onExportsNativeType(LuaJavaEnv::getExportsNativeTypeHandler());

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

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_setGlobal
        (JNIEnv *env, jclass type, jint contextNativeId, jstring name_, jobject value)
{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(contextNativeId);
    if (context != NULL)
    {
        const char *nameStr = env -> GetStringUTFChars(name_, NULL);
        LuaValue *valueObj = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, value);
        context -> setGlobal(nameStr, valueObj);
        valueObj -> release();
        env -> ReleaseStringUTFChars(name_, nameStr);
    }
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_getGlobal
        (JNIEnv *env, jclass type, jint contextNativeId, jstring name_)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(contextNativeId);
    if (context != NULL)
    {
        const char *nameStr = env -> GetStringUTFChars(name_, NULL);
        LuaValue *value = context -> getGlobal(nameStr);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
        value -> release();
        env -> ReleaseStringUTFChars(name_, nameStr);
    }

    return retObj;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_catchException
        (JNIEnv *env, jclass type, jobject jcontext, jboolean enabled)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        if (enabled)
        {
            //设置异常捕获
            context -> onException(LuaJavaEnv::getExceptionHandler());
        }
        else
        {
            //取消异常捕获
            context -> onException(NULL);
        }

    }
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    evalScript
 * Signature: (ILjava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;
 */
JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_evalScript
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring script, jobject jscriptController)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, jscriptController);

        const char* scriptText = env ->GetStringUTFChars(script, NULL);
        LuaValue *value = context->evalScript(scriptText, scriptController);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
        value -> release();
        env -> ReleaseStringUTFChars(script, scriptText);
    }

    return retObj;

}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_evalScriptFromFile
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring path, jobject jscriptController)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, jscriptController);

        const char* scriptPath = env ->GetStringUTFChars(path, NULL);

        LuaValue *value = context->evalScriptFromFile(scriptPath, scriptController);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
        value -> release();
        env -> ReleaseStringUTFChars(path, scriptPath);

    }

    return retObj;
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_callMethod
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring methodName, jobjectArray arguments, jobject jscriptController)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        LuaArgumentList argumentList;

        if (arguments != NULL)
        {
            jsize length = env->GetArrayLength(arguments);
            for (int i = 0; i < length; ++i)
            {
                jobject item = env->GetObjectArrayElement(arguments, i);
                LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, item);
                if (value != NULL) {
                    argumentList.push_back(value);
                }
                env -> DeleteLocalRef(item);
            }
        }

        LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, jscriptController);

        const char *methodNameStr = env -> GetStringUTFChars(methodName, NULL);
        LuaValue *retValue = context->callMethod(methodNameStr, &argumentList, scriptController);
        retObj = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, retValue);
        retValue -> release();
        env -> ReleaseStringUTFChars(methodName, methodNameStr);

        //释放参数内存
        for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
        argumentList.clear();
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

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_invokeFunction
        (JNIEnv *env, jclass thiz, jobject jcontext, jobject func, jobjectArray arguments, jobject jscriptController)
{
    jobject retObj = NULL;
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        //获取LuaFunction
        LuaValue *value = LuaJavaConverter::convertToLuaValueByJObject(env, context, func);
        if (value != NULL)
        {
            LuaArgumentList argumentList;

            if (arguments != NULL)
            {
                jsize length = env->GetArrayLength(arguments);
                for (int i = 0; i < length; ++i)
                {
                    jobject item = env->GetObjectArrayElement(arguments, i);
                    LuaValue *argValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, item);
                    if (argValue != NULL)
                    {
                        argumentList.push_back(argValue);
                    }
                    env -> DeleteLocalRef(item);
                }
            }

            LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, jscriptController);

            LuaValue *retValue = value -> toFunction() -> invoke(&argumentList, scriptController);
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

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_retainValue(JNIEnv *env, jclass type, jobject jcontext, jobject jvalue)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jvalue);
        context -> retainValue(value);
        value -> release();
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_releaseValue(JNIEnv *env, jclass type, jobject jcontext, jobject jvalue)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jvalue);
        context -> releaseValue(value);
        value -> release();
    }
}

JNIEXPORT jboolean JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_registerType(
        JNIEnv *env,
        jclass thiz,
        jobject jcontext,
        jstring alias,
        jstring typeName,
        jstring parentTypeName,
        jclass type,
        jobjectArray fields,
        jobjectArray instanceMethods,
        jobjectArray classMethods)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        const char *typeNameCStr = env->GetStringUTFChars(typeName, 0);

        const char *aliasCStr = NULL;
        if (alias != NULL)
        {
            aliasCStr = env -> GetStringUTFChars(alias, 0);
        }

        const char *parentTypeNameCStr = NULL;
        if (parentTypeName != NULL)
        {
            parentTypeNameCStr = env->GetStringUTFChars(parentTypeName, 0);
        }

        LuaExportTypeDescriptor *parentTypeDescriptor = NULL;
        if (parentTypeNameCStr != NULL)
        {
            parentTypeDescriptor = context -> getExportsTypeManager() -> getExportTypeDescriptor(parentTypeNameCStr);
        }

        if (parentTypeDescriptor == NULL)
        {
            parentTypeDescriptor = context -> getExportsTypeManager() -> getExportTypeDescriptor("Object");
        }

        std::string typeNameStr = typeNameCStr;
        LuaJavaExportTypeDescriptor *typeDescriptor = new LuaJavaExportTypeDescriptor(typeNameStr, env, type, parentTypeDescriptor);

        //设置类型名称映射
        if (typeDescriptor -> typeName() != typeNameCStr)
        {
            context -> getExportsTypeManager() -> _mappingType("android", typeNameCStr, typeDescriptor -> typeName());
        }

        if (aliasCStr != NULL && typeDescriptor -> typeName() != aliasCStr)
        {
            //如果传入格式不等于导出类型名称，则进行映射操作
            context -> getExportsTypeManager() -> _mappingType("android", typeNameCStr, aliasCStr);
        }

        //注册字段
        int fieldsLen = env -> GetArrayLength(fields);
        for (int i = 0; i < fieldsLen; ++i)
        {
            jstring fieldName = (jstring)env -> GetObjectArrayElement(fields, i);
            const char *fieldNameCStr = env -> GetStringUTFChars(fieldName, 0);

            std::deque<std::string> fieldComps =  StringUtils::split(fieldNameCStr, "_", false);

            bool canRead = false;
            bool canWrite = false;

            if (fieldComps[1] == "r")
            {
                canRead = true;
            }
            else if (fieldComps[1] == "w")
            {
                canWrite = true;
            }
            else
            {
                canRead = true;
                canWrite = true;
            }

            LuaJavaExportPropertyDescriptor *propertyDescriptor = new LuaJavaExportPropertyDescriptor(fieldComps[0], canRead, canWrite);
            typeDescriptor -> addProperty(propertyDescriptor -> name(), propertyDescriptor);
            propertyDescriptor -> release();

            fieldComps.clear();

            env -> ReleaseStringUTFChars(fieldName, fieldNameCStr);
            env -> DeleteLocalRef(fieldName);
        }

        //注册实例方法
        int instanceMethodsLen = env -> GetArrayLength(instanceMethods);
        for (int i = 0; i < instanceMethodsLen; ++i)
        {
            jstring methodName = (jstring)env -> GetObjectArrayElement(instanceMethods, i);
            const char *methodNameCStr = env -> GetStringUTFChars(methodName, 0);

            std::deque<std::string> methodComps =  StringUtils::split(methodNameCStr, "_", false);

            LuaJavaExportMethodDescriptor *methodDescriptor = new LuaJavaExportMethodDescriptor(methodComps[0], methodComps[1], LuaJavaMethodTypeInstance);
            typeDescriptor -> addInstanceMethod(methodComps[0], methodDescriptor);
            methodDescriptor -> release();

            methodComps.clear();

            env -> ReleaseStringUTFChars(methodName, methodNameCStr);
            env -> DeleteLocalRef(methodName);
        }

        //注册类方法
        int classMethodsLen = env -> GetArrayLength(classMethods);
        for (int i = 0; i < classMethodsLen; ++i)
        {
            jstring methodName = (jstring)env -> GetObjectArrayElement(classMethods, i);
            const char *methodNameCStr = env -> GetStringUTFChars(methodName, 0);

            std::deque<std::string> methodComps =  StringUtils::split(methodNameCStr, "_", false);

            LuaJavaExportMethodDescriptor *methodDescriptor = new LuaJavaExportMethodDescriptor(methodComps[0], methodComps[1], LuaJavaMethodTypeStatic);
            typeDescriptor -> addClassMethod(methodComps[0], methodDescriptor);
            methodDescriptor -> release();

            methodComps.clear();

            env -> ReleaseStringUTFChars(methodName, methodNameCStr);
            env -> DeleteLocalRef(methodName);
        }

        // 导出类型
        context -> getExportsTypeManager() -> exportsType(typeDescriptor);

        typeDescriptor -> release();

        if (aliasCStr != NULL)
        {
            env -> ReleaseStringUTFChars(alias, aliasCStr);
        }

        if (parentTypeNameCStr != NULL)
        {
            env->ReleaseStringUTFChars(parentTypeName, parentTypeNameCStr);
        }

        env->ReleaseStringUTFChars(typeName, typeNameCStr);

        return JNI_TRUE;
    }

    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_raiseException(JNIEnv *env, jclass type, jobject jcontext, jstring message)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        const char *messageCStr = env->GetStringUTFChars(message, 0);
        context -> raiseException(messageCStr);
        env -> ReleaseStringUTFChars(message, messageCStr);
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_runThread(JNIEnv *env, jclass type, jobject jcontext, jobject jhandler, jobjectArray arguments, jobject jscriptController)
{
    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        //获取LuaFunction
        LuaValue *value = LuaJavaConverter::convertToLuaValueByJObject(env, context, jhandler);
        if (value != NULL)
        {
            LuaArgumentList argumentList;

            if (arguments != NULL)
            {
                jsize length = env->GetArrayLength(arguments);
                for (int i = 0; i < length; ++i)
                {
                    jobject item = env->GetObjectArrayElement(arguments, i);
                    LuaValue *argValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, item);
                    if (argValue != NULL)
                    {
                        argumentList.push_back(argValue);
                    }
                    env -> DeleteLocalRef(item);
                }
            }

            LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, jscriptController);

            context -> runThread(value -> toFunction(), argumentList, scriptController);

            //释放参数内存
            for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; ++it)
            {
                LuaValue *item = *it;
                item -> release();
            }

            value -> release();
        }
    }
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_createScriptController(JNIEnv *env, jclass type)
{
    LuaScriptController *scriptController = new LuaScriptController();
    jobject jScriptController = LuaJavaEnv::createJavaLuaScriptController(env, scriptController);
    scriptController -> release();

    return jScriptController;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_scriptControllerSetTimeout(
        JNIEnv *env,
        jclass type,
        jobject controller,
        jint timeout)
{
    LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, controller);
    if (scriptController != NULL)
    {
        scriptController -> setTimeout(timeout);
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_scriptControllerForceExit(
        JNIEnv *env,
        jclass type,
        jobject controller)
{
    LuaScriptController *scriptController = LuaJavaConverter::convertToScriptControllerByJScriptController(env, controller);
    if (scriptController != NULL)
    {
        scriptController -> forceExit();
    }
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_luaValueSetObject(
        JNIEnv *env,
        jclass type,
        jobject jcontext,
        jobject jvalue,
        jstring keyPath,
        jobject jObject)
{

    jobject retObject = NULL;
    const char *keyPathCStr = env -> GetStringUTFChars(keyPath, 0);

    LuaContext *context = LuaJavaConverter::convertToContextByJLuaContext(env, jcontext);
    if (context != NULL)
    {
        LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jvalue);
        LuaValue *object = NULL;
        if (jObject != NULL)
        {
            object = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jObject);
        }

        if (value != NULL)
        {
            value -> setObject(keyPathCStr, object, context);
            retObject = LuaJavaConverter::convertToJavaObjectByLuaValue(env, context, value);

            value -> release();
        }

        if (object != NULL)
        {
            object -> release();
        }
    }


    env -> ReleaseStringUTFChars(keyPath, keyPathCStr);
    env -> DeleteLocalRef(keyPath);

    return retObject;
}