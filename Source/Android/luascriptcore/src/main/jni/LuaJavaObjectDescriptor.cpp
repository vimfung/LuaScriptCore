//
// Created by 冯鸿杰 on 16/10/31.
//

#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaJavaType.h"
#include "LuaObjectClass.h"
#include "LuaPointer.h"
#include "lua.hpp"

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