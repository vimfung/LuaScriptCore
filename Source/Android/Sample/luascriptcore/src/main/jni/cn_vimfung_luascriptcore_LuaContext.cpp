//
// Created by vimfung on 16/8/22.
//

#include "cn_vimfung_luascriptcore_LuaContext.h"
#include "LuaContext.h"
#include <map>

using namespace cn::vimfung::luascriptcore;

typedef std::map<jstring, LuaContext*> LuaContextMap;
LuaContextMap contexts;

LuaContext* getContextByName(jstring name)
{
    LuaContextMap::iterator it = contexts.find(name);
    if (it != contexts.end())
    {
        return it->second;
    }

    return NULL;
}


jobject convertLuaValueToJLuaValue (LuaValue *value)
{
    return NULL;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaContext_createContext (JNIEnv * env, jobject obj, jstring name)
{
    LuaContextMap::iterator it = contexts.find(name);
    if (it == contexts.end())
    {
        //创建Lua上下文对象
        contexts[name] = new LuaContext();
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaContext_releaseContext (JNIEnv * env, jobject obj, jstring name)
{
    LuaContextMap::iterator it = contexts.find(name);
    if (it != contexts.end())
    {
        it -> second -> release();
        contexts.erase(it);
    }
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaContext_evalScript (JNIEnv *env, jobject obj, jstring contextName, jstring script)
{
    jobject retObj = NULL;
    LuaContext *context = getContextByName(contextName);
    if (context != NULL)
    {
        const char* scriptText = env ->GetStringUTFChars(script, NULL);

        LuaValue *value = context->evalScript(scriptText);
        retObj = convertLuaValueToJLuaValue(value);
        value -> release();
    }
    return retObj;
}