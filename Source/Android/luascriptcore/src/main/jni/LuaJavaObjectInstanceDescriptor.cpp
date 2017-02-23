//
// Created by 冯鸿杰 on 17/1/13.
//

#include "LuaJavaObjectInstanceDescriptor.h"
#include "LuaJavaObjectClass.h"
#include "LuaJavaEnv.h"
#include "LuaContext.h"
#include "LuaJavaType.h"

LuaJavaObjectInstanceDescriptor::LuaJavaObjectInstanceDescriptor(JNIEnv *env, jobject object, LuaJavaObjectClass *objectClass)
    : LuaObjectInstanceDescriptor((const void *)env -> NewGlobalRef(object), objectClass)
{

}


LuaJavaObjectInstanceDescriptor::~LuaJavaObjectInstanceDescriptor()
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    //移除引用
    env -> DeleteGlobalRef((jobject)getObject());
    LuaJavaEnv::resetEnv(env);
}

//void LuaJavaObjectInstanceDescriptor::push(cn::vimfung::luascriptcore::LuaContext *context)
//{
//    JNIEnv *env = LuaJavaEnv::getEnv();
//
//    jobject obj = (jobject)getObject();
//    LuaJavaObjectClass *objectClass = LuaJavaEnv::getObjectClassByInstance(env, obj, context);
//    objectClass -> push(this);
//
//    LuaJavaEnv::resetEnv(env);
//}

bool LuaJavaObjectInstanceDescriptor::instanceOf (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass)
{
    LuaJavaObjectClass *checkClass = dynamic_cast<LuaJavaObjectClass *> (objectClass);
    if (checkClass != NULL)
    {
        JNIEnv *env = LuaJavaEnv::getEnv();

        jobject obj = (jobject)getObject();
        jboolean flag = env -> IsInstanceOf(obj, checkClass -> getModuleClass(env));

        LuaJavaEnv::resetEnv(env);

        return flag;
    }

    return  false;
}