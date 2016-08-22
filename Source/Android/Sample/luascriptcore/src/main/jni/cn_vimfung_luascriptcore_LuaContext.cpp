//
// Created by vimfung on 16/8/22.
//

#include "cn_vimfung_luascriptcore_LuaContext.h"
#include "LuaNativeContext.h"
#include <map>

std::map<jstring, LuaNativeContext*> context;

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaContext_createContext (JNIEnv * env, jobject obj, jstring name)
{

}