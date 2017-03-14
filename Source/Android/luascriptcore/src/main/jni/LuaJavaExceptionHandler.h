//
// Created by 冯鸿杰 on 17/1/13.
//

#ifndef ANDROID_LUAJAVAEXCEPTIONHANDLER_H
#define ANDROID_LUAJAVAEXCEPTIONHANDLER_H

#include <jni.h>

/**
 * Java异常处理器
 */
class LuaJavaExceptionHandler
{
private:
    jobject _jcontext;

public:
    LuaJavaExceptionHandler(JNIEnv *env, jobject jcontext);
    virtual ~LuaJavaExceptionHandler();
};


#endif //ANDROID_LUAJAVAEXCEPTIONHANDLER_H
