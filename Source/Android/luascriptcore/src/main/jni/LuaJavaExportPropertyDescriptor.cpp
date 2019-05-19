//
// Created by 冯鸿杰 on 2017/12/1.
//

#include "LuaJavaExportPropertyDescriptor.h"
#include "LuaSession.h"
#include "LuaJavaEnv.h"
#include "LuaJavaType.h"
#include "LuaJavaExportTypeDescriptor.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaConverter.h"
#include <jni.h>


LuaJavaExportPropertyDescriptor::LuaJavaExportPropertyDescriptor(std::string name,
                                                                 bool canRead,
                                                                 bool canWrite)
    : LuaExportPropertyDescriptor(name, canRead, canWrite)
{

}

LuaValue* LuaJavaExportPropertyDescriptor::invokeGetter(LuaSession *session, LuaObjectDescriptor *instance)
{
    if (canRead())
    {
        LuaContext *context = session -> getContext();

        JNIEnv *env = LuaJavaEnv::getEnv();

        jobject jExportTypeManager = LuaJavaEnv::getExportTypeManager(env);
        jmethodID invokeMethodId = env -> GetMethodID(LuaJavaType::exportTypeManagerClass(env), "getterMethodRoute", "(Lcn/vimfung/luascriptcore/LuaContext;Ljava/lang/Object;Ljava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;");

        jobject jContext = LuaJavaEnv::getJavaLuaContext(env, context);
        jstring methodName = LuaJavaEnv::newString(env, name());

        LuaJavaObjectDescriptor *objectDescriptor = (LuaJavaObjectDescriptor *)instance;
        jobject jReturnValue = env -> CallObjectMethod(jExportTypeManager, invokeMethodId, jContext, objectDescriptor -> getJavaObject(), methodName);

        env -> DeleteLocalRef(methodName);

        LuaValue *returnValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jReturnValue);

        env -> DeleteLocalRef(jReturnValue);

        LuaJavaEnv::resetEnv(env);

        return returnValue;
    }

    return NULL;

}


void LuaJavaExportPropertyDescriptor::invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value)
{
    if (canWrite())
    {
        LuaContext *context = session -> getContext();

        JNIEnv *env = LuaJavaEnv::getEnv();

        jobject jExportTypeManager = LuaJavaEnv::getExportTypeManager(env);
        jmethodID invokeMethodId = env -> GetMethodID(LuaJavaType::exportTypeManagerClass(env), "setterMethodRoute", "(Lcn/vimfung/luascriptcore/LuaContext;Ljava/lang/Object;Ljava/lang/String;Lcn/vimfung/luascriptcore/LuaValue;)V");

        jobject jContext = LuaJavaEnv::getJavaLuaContext(env, context);
        jstring methodName = LuaJavaEnv::newString(env, name());

        LuaJavaObjectDescriptor *objectDescriptor = (LuaJavaObjectDescriptor *)instance;

        jobject jLuaValue = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
        env -> CallVoidMethod(jExportTypeManager, invokeMethodId, jContext, objectDescriptor -> getJavaObject(), methodName, jLuaValue);

        env -> DeleteLocalRef(jLuaValue);
        env -> DeleteLocalRef(methodName);

        LuaJavaEnv::resetEnv(env);
    }
}