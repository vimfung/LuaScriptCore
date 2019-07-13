//
// Created by 冯鸿杰 on 2017/11/16.
//

#include <StringUtils.h>
#include "LuaJavaExportMethodDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaJavaType.h"
#include "LuaJavaConverter.h"
#include "LuaJavaExportTypeDescriptor.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaDefine.h"

LuaJavaExportMethodDescriptor::LuaJavaExportMethodDescriptor(std::string name, std::string methodSignature, LuaJavaMethodType type)
    : LuaExportMethodDescriptor(name, methodSignature)
{
    _type = type;
}

LuaValue* LuaJavaExportMethodDescriptor::invoke(LuaSession *session, LuaArgumentList arguments)
{
    switch (_type)
    {
        case LuaJavaMethodTypeStatic:
            //类方法
            return invokeClassMethod(session, arguments);
        case LuaJavaMethodTypeInstance:
            //实例方法
            return invokeInstanceMethod(session, arguments);
        default:
            return NULL;
    }
}

LuaValue* LuaJavaExportMethodDescriptor::invokeClassMethod(LuaSession *session, LuaArgumentList arguments)
{
    LuaContext *context = session -> getContext();

    JNIEnv *env = LuaJavaEnv::getEnv();

    jobject jExportTypeManager = LuaJavaEnv::getExportTypeManager(env);
    jmethodID invokeMethodId = env -> GetMethodID(LuaJavaType::exportTypeManagerClass(env), "classMethodRoute", "(Lcn/vimfung/luascriptcore/LuaContext;Ljava/lang/Class;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");

    jobject jContext = LuaJavaEnv::getJavaLuaContext(env, context);

    std::string methodNameString = StringUtils::format("%s_%s", name().c_str(), methodSignature().c_str());

    jstring methodName = LuaJavaEnv::newString(env, methodNameString);

    int index = 0;
    jobjectArray jArgs = env -> NewObjectArray((jsize)arguments.size(), LuaJavaType::luaValueClass(env), NULL);
    for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end() ; ++it)
    {
        LuaValue *argItem = *it;
        jobject jArgItem = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, argItem);
        env -> SetObjectArrayElement(jArgs, index, jArgItem);
        env -> DeleteLocalRef(jArgItem);

        index++;
    }

    LuaJavaExportTypeDescriptor *javaTypeDescriptor = (LuaJavaExportTypeDescriptor *)typeDescriptor;
    jobject jReturnValue = env -> CallObjectMethod(jExportTypeManager, invokeMethodId, jContext, javaTypeDescriptor -> getJavaType(), methodName, jArgs);

    env -> DeleteLocalRef(methodName);
    env -> DeleteLocalRef(jArgs);

    LuaValue *returnValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jReturnValue);

    env -> DeleteLocalRef(jReturnValue);

    LuaJavaEnv::resetEnv(env);

    return returnValue;
}

LuaValue* LuaJavaExportMethodDescriptor::invokeInstanceMethod(LuaSession *session, LuaArgumentList arguments)
{
    LuaContext *context = session -> getContext();

    JNIEnv *env = LuaJavaEnv::getEnv();

    jobject jExportTypeManager = LuaJavaEnv::getExportTypeManager(env);
    jmethodID invokeMethodId = env -> GetMethodID(LuaJavaType::exportTypeManagerClass(env), "instanceMethodRoute", "(Lcn/vimfung/luascriptcore/LuaContext;Ljava/lang/Object;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
    jobject jContext = LuaJavaEnv::getJavaLuaContext(env, context);

    std::string methodNameString = StringUtils::format("%s_%s", name().c_str(), methodSignature().c_str());
    jstring methodName = LuaJavaEnv::newString(env, methodNameString);

    LuaArgumentList::iterator it = arguments.begin();
    LuaJavaObjectDescriptor *objectDescriptor = (LuaJavaObjectDescriptor *)((*it) -> toObject());
    it ++;

    int index = 0;
    jobjectArray jArgs = env -> NewObjectArray((jsize)arguments.size() - 1, LuaJavaType::luaValueClass(env), NULL);
    for (; it != arguments.end() ; ++it)
    {
        LuaValue *argItem = *it;
        jobject jArgItem = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, argItem);
        env -> SetObjectArrayElement(jArgs, index, jArgItem);
        env -> DeleteLocalRef(jArgItem);

        index++;
    }

    jobject jReturnValue = env -> CallObjectMethod(jExportTypeManager, invokeMethodId, jContext, objectDescriptor -> getJavaObject(), methodName, jArgs);

    env -> DeleteLocalRef(methodName);
    env -> DeleteLocalRef(jArgs);

    LuaValue *returnValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, jReturnValue);

    env -> DeleteLocalRef(jReturnValue);

    LuaJavaEnv::resetEnv(env);

    return returnValue;
}