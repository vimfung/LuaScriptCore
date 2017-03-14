//
// Created by 冯鸿杰 on 17/1/13.
//

#include "LuaJavaExceptionHandler.h"
#include "LuaJavaEnv.h"

LuaJavaExceptionHandler::LuaJavaExceptionHandler(JNIEnv *env, jobject jcontext)
    : _jcontext(env -> NewWeakGlobalRef(jcontext))
{

}

LuaJavaExceptionHandler::~LuaJavaExceptionHandler()
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    env -> DeleteWeakGlobalRef(_jcontext);

    LuaJavaEnv::resetEnv(env);
}