//
// Created by 冯鸿杰 on 2017/11/16.
//

#include "LuaJavaExportTypeDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaJavaObjectDescriptor.h"

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

    //创建实例对象
    jclass objType = getJavaType();
    jmethodID initMethodId = env->GetMethodID(objType, "<init>", "()V");
    jobject jInstance = env->NewObject(objType, initMethodId);

    LuaJavaObjectDescriptor *objectDescriptor = new LuaJavaObjectDescriptor(env, jInstance, this);

    env -> DeleteLocalRef(jInstance);

    LuaJavaEnv::resetEnv(env);

    return objectDescriptor;
}

void LuaJavaExportTypeDescriptor::destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor)
{
    LuaExportTypeDescriptor::destroyInstance(session, objectDescriptor);
}