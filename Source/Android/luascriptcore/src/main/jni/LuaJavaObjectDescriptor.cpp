//
// Created by 冯鸿杰 on 16/10/31.
//

#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaJavaType.h"
#include "LuaObjectClass.h"
#include "LuaPointer.h"
#include "../../../../../lua-core/src/lua.hpp"

LuaJavaObjectDescriptor::LuaJavaObjectDescriptor(JNIEnv *env, jobject object)
{
    //添加引用
    setObject((const void *)env -> NewGlobalRef(object));
}

LuaJavaObjectDescriptor::~LuaJavaObjectDescriptor()
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    //移除引用
    env -> DeleteGlobalRef((jobject)getObject());
    LuaJavaEnv::resetEnv(env);
}

void LuaJavaObjectDescriptor::push(LuaContext *context)
{
    bool process = false;

    JNIEnv *env = LuaJavaEnv::getEnv();

    jobject obj = (jobject)getObject();
    if (env -> IsInstanceOf(obj, LuaJavaType::luaObjectClass(env)))
    {
        LuaJavaObjectClass *objectClass = LuaJavaEnv::getObjectClassByInstance(env, obj, context);
        objectClass -> push(this);
        process = true;

    }

    LuaJavaEnv::resetEnv(env);

    if (!process)
    {
        //调用父类方法
        LuaObjectDescriptor::push(context);
    }

}