//
// Created by 冯鸿杰 on 2017/11/16.
//

#include <LuaEngineAdapter.hpp>
#include "LuaJavaExportTypeDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaSession.h"
#include "LuaJavaType.h"
#include "LuaJavaConverter.h"

LuaJavaExportTypeDescriptor::LuaJavaExportTypeDescriptor (std::string &typeName, JNIEnv *env, jclass jType, LuaExportTypeDescriptor *parentTypeDescriptor)
    : LuaExportTypeDescriptor(typeName, parentTypeDescriptor)
{
    _jType = (jclass)env -> NewWeakGlobalRef(jType);
}

LuaJavaExportTypeDescriptor::~LuaJavaExportTypeDescriptor()
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    env -> DeleteWeakGlobalRef((jobject)_jType);
    LuaJavaEnv::resetEnv(env);
}

jclass LuaJavaExportTypeDescriptor::getJavaType()
{
    return _jType;
}

LuaObjectDescriptor* LuaJavaExportTypeDescriptor::createInstance(LuaSession *session)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    //获取传入参数
    LuaArgumentList args;
    session -> parseArguments(args, 2);

    jobject jExportTypeManager = LuaJavaEnv::getExportTypeManager(env);
    jclass jExportTypeManagerCls = LuaJavaType::exportTypeManagerClass(env);
    jmethodID invokeMethodId = env -> GetMethodID(jExportTypeManagerCls, "constructorMethodRoute", "(Lcn/vimfung/luascriptcore/LuaContext;Ljava/lang/Class;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");

    jobject jContext = LuaJavaEnv::getJavaLuaContext(env, session -> getContext());


    int index = 0;
    jobjectArray jArgs = env -> NewObjectArray((jsize)args.size(), LuaJavaType::luaValueClass(env), NULL);
    for (LuaArgumentList::iterator it = args.begin(); it != args.end(); ++it)
    {
        LuaValue *argItem = *it;
        jobject jArgItem = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, session -> getContext(), argItem);
        env -> SetObjectArrayElement(jArgs, index, jArgItem);
        env -> DeleteLocalRef(jArgItem);

        index++;
    }

    jobject jReturnValue = env -> CallObjectMethod(jExportTypeManager, invokeMethodId, jContext, this -> getJavaType(), jArgs);

    env -> DeleteLocalRef(jArgs);

    LuaJavaObjectDescriptor *objectDescriptor = NULL;
    LuaValue *returnValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, session -> getContext(), jReturnValue);
    env -> DeleteLocalRef(jReturnValue);

    if (returnValue -> getType() != LuaValueTypeNil)
    {
        objectDescriptor = dynamic_cast<LuaJavaObjectDescriptor *>(returnValue -> toObject());
        objectDescriptor -> retain();
    }
    else
    {
        session -> reportLuaException("Unsupported constructor method");
    }

    //释放参数对象
    for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
    {
        LuaValue *value = *it;
        value -> release();
    }

    returnValue -> release();

    LuaJavaEnv::resetEnv(env);

    return objectDescriptor;
}

void LuaJavaExportTypeDescriptor::destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor)
{
    LuaExportTypeDescriptor::destroyInstance(session, objectDescriptor);
}