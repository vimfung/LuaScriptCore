//
// Created by vimfung on 16/8/22.
//

#include "cn_vimfung_luascriptcore_LuaContext.h"
#include "LuaContext.h"
#include "LuaDefine.h"
#include <map>
#include <string>

using namespace cn::vimfung::luascriptcore;

typedef std::map<std::string, LuaContext*> LuaContextMap;
LuaContextMap contexts;

LuaContext* getContextByName(std::string name)
{
    LuaContextMap::iterator it = contexts.find(name);
    if (it != contexts.end())
    {
        LOGI("find context!");
        return it->second;
    }

    return NULL;
}

jobject convertLuaValueToJObject (JNIEnv *env, LuaValue *value)
{
    jobject retObj = NULL;
    if (value != NULL)
    {
        switch (value->getType())
        {
            case LuaValueTypeNumber:
            {
                static jclass jDoubleClass = env -> FindClass("java/lang/Double");
                static jmethodID initMethodId = env -> GetMethodID(jDoubleClass, "<init>", "(D)V");

                retObj = env -> NewObject(jDoubleClass, initMethodId, value -> toNumber());

                break;
            }
            case LuaValueTypeBoolean:
            {
                static jclass jBooleanClass = env -> FindClass("java/lang/Boolean");
                static jmethodID initMethodId = env -> GetMethodID(jBooleanClass, "<init>", "(Z)V");

                retObj = env -> NewObject(jBooleanClass, initMethodId, value -> toBoolean());

                break;
            }
            case LuaValueTypeString:
            {
                retObj = env->NewStringUTF(value->toString().c_str());
                break;
            }
            case LuaValueTypeData:
            {
                jsize size = (jsize)value -> getDataLength();
                jbyteArray byteArray = env -> NewByteArray(size);
                env -> SetByteArrayRegion(byteArray, 0, size, (const jbyte *)value -> toData());

                retObj = byteArray;

                break;
            }
            case LuaValueTypeArray:
            {
                LuaValueList *list = value -> toArray();
                if (list != NULL) {
                    static jclass jArrayListClass = env->FindClass("java/util/ArrayList");
                    static jmethodID initMethodId = env->GetMethodID(jArrayListClass, "<init>",
                                                                     "()V");
                    static jmethodID addMethodId = env->GetMethodID(jArrayListClass, "add",
                                                                    "(Ljava/lang/Object;)Z");

                    retObj = env->NewObject(jArrayListClass, initMethodId);
                    for (LuaValueList::iterator i = list->begin(); i != list->end(); ++i) {
                        LuaValue *item = *i;
                        jobject itemObj = convertLuaValueToJObject(env, item);
                        if (itemObj != NULL) {
                            env->CallObjectMethod(retObj, addMethodId, itemObj);
                        }
                    }
                }
                break;
            }
            case LuaValueTypeMap:
            {
                LuaValueMap *map = value -> toMap();
                if (map != NULL)
                {
                    static jclass jHashMapClass = env -> FindClass("java/util/HashMap");
                    static jmethodID initMethodId = env -> GetMethodID(jHashMapClass, "<init>", "()V");
                    static jmethodID putMethodId = env -> GetMethodID(jHashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

                    retObj = env -> NewObject(jHashMapClass, initMethodId);
                    for (LuaValueMap::iterator i = map -> begin(); i != map -> end() ; ++i)
                    {
                        std::string key = i -> first;
                        LuaValue *item = i -> second;

                        jstring keyStr = env -> NewStringUTF(key.c_str());
                        jobject itemObj = convertLuaValueToJObject(env, item);
                        if (keyStr != NULL && itemObj != NULL)
                        {
                            env -> CallObjectMethod(retObj, putMethodId, keyStr, itemObj);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return retObj;
}

jobject convertLuaValueToJLuaValue (JNIEnv *env, LuaValue *value)
{
    jobject retObj = NULL;
    if (value != NULL)
    {
        static jclass jLuaValue = env -> FindClass("cn/vimfung/luascriptcore/LuaValue");
        jmethodID initMethodId = env -> GetMethodID(jLuaValue, "<init>", "()V");

        switch (value->getType())
        {
            case LuaValueTypeNumber:
            {
                static jmethodID numberInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(D)V");
                initMethodId = numberInitMethodId;
                break;
            }
            case LuaValueTypeBoolean:
            {
                static jmethodID boolInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(Z)V");
                initMethodId = boolInitMethodId;
                break;
            }
            case LuaValueTypeString:
            {
                static jmethodID stringInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(Ljava/lang/String;)V");
                initMethodId = stringInitMethodId;
                break;
            }
            case LuaValueTypeData:
            {
                static jmethodID byteArrInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "([B)V");
                initMethodId = byteArrInitMethodId;
                break;
            }
            case LuaValueTypeArray:
            {
                static jmethodID arrayInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(Ljava/util/ArrayList;)V");
                initMethodId = arrayInitMethodId;
                break;
            }
            case LuaValueTypeMap:
            {
                static jmethodID mapInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(Ljava/util/HashMap;)V");
                initMethodId = mapInitMethodId;
                break;
            }
            default:
                break;
        }

        retObj = env -> NewObject(jLuaValue, initMethodId, convertLuaValueToJObject(env, value));
    }

    return retObj;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaContext_createContext (JNIEnv * env, jobject obj, jstring name)
{
    printf("Hello World!!!!!!!!\n");

    std::string contextName = env -> GetStringUTFChars(name, NULL);
    LuaContextMap::iterator it = contexts.find(contextName);
    if (it == contexts.end())
    {
        LOGI("new context create");

        //创建Lua上下文对象
        contexts[contextName] = new LuaContext();
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaContext_releaseContext (JNIEnv * env, jobject obj, jstring name)
{
    std::string contextName = env -> GetStringUTFChars(name, NULL);
    LuaContextMap::iterator it = contexts.find(contextName);
    if (it != contexts.end())
    {
        it -> second -> release();
        contexts.erase(it);
    }
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaContext_evalScript (JNIEnv *env, jobject obj, jstring contextName, jstring script)
{
    LOGI("start eval script %s, %s", env -> GetStringUTFChars(contextName, NULL), env -> GetStringUTFChars(script, NULL));

    jobject retObj = NULL;
    std::string contextNameStr = env -> GetStringUTFChars(contextName, NULL);
    LuaContext *context = getContextByName(contextNameStr);
    if (context != NULL)
    {
        const char* scriptText = env ->GetStringUTFChars(script, NULL);

        LuaValue *value = context->evalScript(scriptText);
        retObj = convertLuaValueToJLuaValue(env, value);
        value -> release();
    }

    return retObj;
}