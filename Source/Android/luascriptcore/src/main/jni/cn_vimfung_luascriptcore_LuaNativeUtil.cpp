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
#include "LuaExportsTypeManager.hpp"
#include "StringUtils.h"

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
        (JNIEnv *env, jclass obj, jobject config)
{
    LuaContext *context = new LuaContext();
    jobject jcontext = LuaJavaEnv::createJavaLuaContext(env, context, config);
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
            context -> onException(LuaJavaEnv::getExceptionhandler());
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

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_invokeFunction
        (JNIEnv *env, jclass thiz, jobject jcontext, jobject func, jobjectArray arguments)
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
                    LuaValue *value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, item);
                    if (value != NULL)
                    {
                        argumentList.push_back(value);
                    }
                    env -> DeleteLocalRef(item);
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
        jboolean lazyImport,
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
        const char *parentTypeNameCStr = env->GetStringUTFChars(parentTypeName, 0);

        LuaExportTypeDescriptor *parentTypeDescriptor = NULL;
        if (parentTypeNameCStr != NULL)
        {
            parentTypeDescriptor = context -> getExportsTypeManager() -> getExportTypeDescriptor(parentTypeNameCStr);
        }

        std::string typeNameStr = typeNameCStr;
        LuaJavaExportTypeDescriptor *typeDescriptor = new LuaJavaExportTypeDescriptor(typeNameStr, env, type, parentTypeDescriptor);

        //注册字段
        int fieldsLen = env -> GetArrayLength(fields);
        for (int i = 0; i < fieldsLen; ++i)
        {
            jstring fieldName = (jstring)env -> GetObjectArrayElement(fields, i);
            const char *fieldNameCStr = env -> GetStringUTFChars(fieldName, 0);

            std::vector<std::string> fieldComps =  StringUtils::split(fieldNameCStr, "_", false);

            //添加Getter
            LuaJavaExportMethodDescriptor *getterMethodDescriptor = new LuaJavaExportMethodDescriptor(fieldComps[0], fieldComps[1], LuaJavaMethodTypeGetter);
            typeDescriptor -> addInstanceMethod(fieldComps[0], getterMethodDescriptor);
            getterMethodDescriptor -> release();

            //添加Setter
            //与iOS中的属性getter和setter方法保持一致, getter直接是属性名称,setter则需要将属性首字母大写并在前面加上set
            char upperCStr[2] = {0};
            upperCStr[0] = (char)toupper(fieldComps[0][0]);
            std::string upperStr = upperCStr;
            std::string fieldNameStr = fieldComps[0].c_str() + 1;
            std::string setterMethodName = "set" + upperStr + fieldNameStr;
            LuaJavaExportMethodDescriptor *setterMethodDescriptor = new LuaJavaExportMethodDescriptor(fieldComps[0], fieldComps[1], LuaJavaMethodTypeSetter);
            typeDescriptor -> addInstanceMethod(setterMethodName, setterMethodDescriptor);
            setterMethodDescriptor -> release();

            env -> ReleaseStringUTFChars(fieldName, fieldNameCStr);
        }

        //注册实例方法
        int instanceMethodsLen = env -> GetArrayLength(instanceMethods);
        for (int i = 0; i < instanceMethodsLen; ++i)
        {
            jstring methodName = (jstring)env -> GetObjectArrayElement(instanceMethods, i);
            const char *methodNameCStr = env -> GetStringUTFChars(methodName, 0);

            std::vector<std::string> methodComps =  StringUtils::split(methodNameCStr, "_", false);

            LuaJavaExportMethodDescriptor *methodDescriptor = new LuaJavaExportMethodDescriptor(methodComps[0], methodComps[1], LuaJavaMethodTypeInstance);
            typeDescriptor -> addInstanceMethod(methodComps[0], methodDescriptor);
            methodDescriptor -> release();

            env -> ReleaseStringUTFChars(methodName, methodNameCStr);
        }

        //注册类方法
        int classMethodsLen = env -> GetArrayLength(classMethods);
        for (int i = 0; i < classMethodsLen; ++i)
        {
            jstring methodName = (jstring)env -> GetObjectArrayElement(classMethods, i);
            const char *methodNameCStr = env -> GetStringUTFChars(methodName, 0);

            std::vector<std::string> methodComps =  StringUtils::split(methodNameCStr, "_", false);

            LuaJavaExportMethodDescriptor *methodDescriptor = new LuaJavaExportMethodDescriptor(methodComps[0], methodComps[1], LuaJavaMethodTypeStatic);
            typeDescriptor -> addClassMethod(methodComps[0], methodDescriptor);
            methodDescriptor -> release();

            env -> ReleaseStringUTFChars(methodName, methodNameCStr);
        }

        // 导出类型
        context -> getExportsTypeManager() -> exportsType(typeDescriptor, lazyImport);

        typeDescriptor -> release();

        env->ReleaseStringUTFChars(typeName, typeNameCStr);
        env->ReleaseStringUTFChars(parentTypeName, parentTypeNameCStr);

        return JNI_TRUE;
    }

    return JNI_FALSE;
}