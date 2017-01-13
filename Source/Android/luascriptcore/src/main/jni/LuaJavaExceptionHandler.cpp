//
// Created by 冯鸿杰 on 17/1/13.
//

#include "LuaJavaExceptionHandler.h"

LuaJavaExceptionHandler::LuaJavaExceptionHandler(JNIEnv *env, jobject jcontext)
    : _jcontext(env -> NewWeakGlobalRef(jcontext))
{

}