//
// Created by vimfung on 16/8/29.
//
#include <stddef.h>
#include "cn_vimfung_luascriptcore_LuaNativeUtil.h"

#include "LuaContext.h"
#include "LuaObjectManager.h"
#include "LuaDefine.h"

using namespace cn::vimfung::luascriptcore;

/**
 * 获取Java层中的LuaValue类
 */
jclass getJLuaValueClass(JNIEnv *env)
{
    static jclass jLuaValue = NULL;

    if (jLuaValue == NULL)
    {
        jclass jLuaValueCls = env -> FindClass("cn/vimfung/luascriptcore/LuaValue");
        jLuaValue = (jclass)env -> NewGlobalRef(jLuaValueCls);
        env -> DeleteLocalRef(jLuaValueCls);
    }

    return jLuaValue;
}

/**
 * 获取Java层中的ArrayList类型
 */
jclass getJArrayListClass(JNIEnv *env)
{
    static jclass jArrayList = NULL;

    if (jArrayList == NULL)
    {
        jclass  jArrayListCls = env -> FindClass("java/util/ArrayList");
        jArrayList = (jclass)env -> NewGlobalRef(jArrayListCls);
        env -> DeleteLocalRef(jArrayListCls);
    }

    return jArrayList;
}

/**
 * 获取Java层中的HashMap类型
 */
jclass getJHashMapClass(JNIEnv *env)
{
    static jclass jHashMap = NULL;

    if (jHashMap == NULL)
    {
        jclass jHashMapCls = env -> FindClass("java/util/HashMap");
        jHashMap = (jclass)env -> NewGlobalRef(jHashMapCls);
        env -> DeleteLocalRef(jHashMapCls);
    }

    return jHashMap;
}

/**
 * 转换LuaValue为Java对象
 * @param env JNI环境对象
 * @param value LuaValue对象
 * @return 转换后的Java对象
 */
jobject convertLuaValueToJObject (JNIEnv *env, LuaValue *value)
{
    jobject retObj = NULL;
    if (value != NULL)
    {
        switch (value->getType())
        {
            case LuaValueTypeNumber:
            {
                static jclass jDoubleClass = (jclass)env -> NewGlobalRef(env -> FindClass("java/lang/Double"));
                static jmethodID initMethodId = env -> GetMethodID(jDoubleClass, "<init>", "(D)V");

                retObj = env -> NewObject(jDoubleClass, initMethodId, value -> toNumber());

                break;
            }
            case LuaValueTypeBoolean:
            {
                static jclass jBooleanClass = (jclass)env -> NewGlobalRef(env -> FindClass("java/lang/Boolean"));
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
                    static jclass jArrayListClass = getJArrayListClass(env);
                    static jmethodID initMethodId = env->GetMethodID(jArrayListClass, "<init>",
                                                                     "()V");
                    static jmethodID addMethodId = env->GetMethodID(jArrayListClass, "add",
                                                                    "(Ljava/lang/Object;)Z");

                    retObj = env->NewObject(jArrayListClass, initMethodId);
                    for (LuaValueList::iterator i = list->begin(); i != list->end(); ++i) {
                        LuaValue *item = *i;
                        jobject itemObj = convertLuaValueToJObject(env, item);
                        if (itemObj != NULL) {
                            env->CallBooleanMethod(retObj, addMethodId, itemObj);
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
                    static jclass jHashMapClass = getJHashMapClass(env);
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

/**
 * 转换LuaValue为Java中的LuaValue
 */
jobject convertLuaValueToJLuaValue (JNIEnv *env, LuaValue *value)
{
    jobject retObj = NULL;
    if (value != NULL)
    {
        static jclass jLuaValue = getJLuaValueClass(env);
        static jmethodID jNilInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(I)V");
        jmethodID initMethodId = jNilInitMethodId;

        switch (value->getType())
        {
            case LuaValueTypeNumber:
            {
                static jmethodID numberInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILjava/lang/Double;)V");
                initMethodId = numberInitMethodId;
                break;
            }
            case LuaValueTypeBoolean:
            {
                static jmethodID boolInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILjava/lang/Boolean;)V");
                initMethodId = boolInitMethodId;
                break;
            }
            case LuaValueTypeString:
            {
                static jmethodID stringInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILjava/lang/String;)V");
                initMethodId = stringInitMethodId;
                break;
            }
            case LuaValueTypeData:
            {
                static jmethodID byteArrInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(I[B)V");
                initMethodId = byteArrInitMethodId;
                break;
            }
            case LuaValueTypeArray:
            {
                static jmethodID arrayInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILjava/util/ArrayList;)V");
                initMethodId = arrayInitMethodId;
                break;
            }
            case LuaValueTypeMap:
            {
                static jmethodID mapInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILjava/util/HashMap;)V");
                initMethodId = mapInitMethodId;
                break;
            }
            default:
                break;
        }

        int objectId = LuaObjectManager::SharedInstance() -> putObject(value);

        if (value -> getType() == LuaValueTypeNil)
        {
            retObj = env -> NewObject(jLuaValue, initMethodId, objectId);
        }
        else
        {
            retObj = env -> NewObject(jLuaValue, initMethodId, objectId, convertLuaValueToJObject(env, value));
        }
    }

    return retObj;
}

LuaValue* convertJLuaValueToLuaValue (JNIEnv *env, jobject value)
{
    //构造调用参数
    static jclass jLuaValueType = (jclass)env -> NewGlobalRef(env -> FindClass("cn/vimfung/luascriptcore/LuaValueType"));
    static jmethodID typeValueMethodId = env -> GetMethodID(jLuaValueType, "value", "()I");

    static jclass jLuaValueClass = getJLuaValueClass(env);
    static jmethodID typeMethodId = env -> GetMethodID(jLuaValueClass, "valueType", "()Lcn/vimfung/luascriptcore/LuaValueType;");
    static jmethodID toIntMethodId = env -> GetMethodID(jLuaValueClass, "toInteger", "()I");
    static jmethodID toNumMethodId = env -> GetMethodID(jLuaValueClass, "toNumber", "()D");
    static jmethodID toBoolMethodId = env -> GetMethodID(jLuaValueClass, "toBoolean", "()Z");
    static jmethodID toStrMethodId = env -> GetMethodID(jLuaValueClass, "toString", "()Ljava/lang/String;");
    static jmethodID toByteArrMethodId = env -> GetMethodID(jLuaValueClass, "toByteArray", "()[B");
    static jmethodID toListMethodId = env -> GetMethodID(jLuaValueClass, "toArrayList", "()Ljava/util/ArrayList;");
    static jmethodID toMapMethodId = env -> GetMethodID(jLuaValueClass, "toHashMap", "()Ljava/util/HashMap;");

    jobject itemType = env -> CallObjectMethod(value, typeMethodId);
    jint valueType = env -> CallIntMethod(itemType, typeValueMethodId);

    LuaValue *retValue = NULL;
    switch (valueType)
    {
        case LuaValueTypeNil:
            retValue = LuaValue::NilValue();
            break;
        case LuaValueTypeInteger:
            retValue = LuaValue::IntegerValue(env -> CallIntMethod(value, toIntMethodId));
            break;
        case LuaValueTypeNumber:
            retValue = LuaValue::NumberValue(env -> CallDoubleMethod(value, toNumMethodId));
            break;
        case LuaValueTypeBoolean:
            retValue = LuaValue::BooleanValue(env -> CallBooleanMethod(value, toBoolMethodId));
            break;
        case LuaValueTypeString:
        {
            jstring strValue = (jstring)env -> CallObjectMethod(value, toStrMethodId);
            const char *charStr = env -> GetStringUTFChars(strValue, NULL);
            retValue = LuaValue::StringValue(charStr);
            break;
        }
        case LuaValueTypeData:
        {
            jbyteArray byteArr = (jbyteArray)env -> CallObjectMethod(value, toByteArrMethodId);
            jsize len = env -> GetArrayLength(byteArr);

            jbyte buffer[len];
            env -> GetByteArrayRegion(byteArr, 0, len, buffer);

            retValue = LuaValue::DataValue((const char *)buffer, (size_t)len);
            break;
        }
        case LuaValueTypeArray:
        {
            static jclass jArrayListClass = getJArrayListClass(env);
            static jmethodID getMethodId = env -> GetMethodID(jArrayListClass, "get", "(I)Ljava/lang/Object");
            static jmethodID sizeMethodId = env -> GetMethodID(jArrayListClass, "size", "()I");

            LuaValueList list;
            jobject arrayList = env -> CallObjectMethod(value, toListMethodId);
            jint len = env -> CallIntMethod(arrayList, sizeMethodId);
            for (int i = 0; i < len; ++i)
            {
                jobject item = env -> CallObjectMethod(arrayList, getMethodId, i);
                LuaValue *valueItem = convertJLuaValueToLuaValue(env, item);
            }

            retValue = LuaValue::ArrayValue(list);
            break;
        }
        case LuaValueTypeMap:
        {
            static jclass jHashMapClass = getJHashMapClass(env);
            static jmethodID getMethodId = env -> GetMethodID(jHashMapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object");
            static jmethodID sizeMethodId = env -> GetMethodID(jHashMapClass, "size", "()I");
            static jmethodID keySetMethodId = env -> GetMethodID(jHashMapClass, "keySet", "()Ljava/util/Set");

            static jclass jSetClass = (jclass)env -> NewGlobalRef(env -> FindClass("java/util/Set"));
            static jmethodID toArrayMethodId = env -> GetMethodID(jSetClass, "toArray", "()[Ljava/lang/Object");

            LuaValueMap map;
            jobject hashMap = env -> CallObjectMethod(value, toMapMethodId);
            jint len = env -> CallIntMethod(hashMap, sizeMethodId);

            jobject keySet= env -> CallObjectMethod(hashMap, keySetMethodId);
            jobjectArray keys = (jobjectArray)env -> CallObjectMethod(keySet, toArrayMethodId);

            for (int i = 0; i < len; ++i)
            {
                jobject key = env -> GetObjectArrayElement(keys, i);
                jobject item = env -> CallObjectMethod(hashMap, getMethodId, key);

                const char *keyStr = env -> GetStringUTFChars((jstring)key, NULL);
                LuaValue *valueItem = convertJLuaValueToLuaValue(env, item);
                map[keyStr] = valueItem;
            }

            retValue = LuaValue::DictonaryValue(map);

            break;
        }
        default:
            break;
    }

    return retValue;
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    createContext
 * Signature: ()Lcn/vimfung/luascriptcore/LuaContext;
 */
JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_createContext
        (JNIEnv *env, jclass obj)
{
    jclass contextClass = env -> FindClass("cn/vimfung/luascriptcore/LuaContext");
    jmethodID initMethodId = env -> GetMethodID(contextClass, "<init>", "(I)V");

    LuaContext *context = new LuaContext();
    int nativeId = LuaObjectManager::SharedInstance() -> putObject(context);
    context -> release();

    return env -> NewObject(contextClass, initMethodId, nativeId);
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    evalScript
 * Signature: (ILjava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;
 */
JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_evalScript
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring script)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char* scriptText = env ->GetStringUTFChars(script, NULL);
        LuaValue *value = context->evalScript(scriptText);
        retObj = convertLuaValueToJLuaValue(env, value);
        value -> release();
    }

    return retObj;

}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_evalScriptFromFile
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring path)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char* scriptPath = env ->GetStringUTFChars(path, NULL);

        LOGI("start eval script from file = %s", scriptPath);
        LuaValue *value = context->evalScriptFromFile(scriptPath);
        LOGI("end eval script from file====== type  ＝ %d", value->getType());

        retObj = convertLuaValueToJLuaValue(env, value);
        value -> release();
    }

    return retObj;
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_callMethod
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring methodName, jobjectArray arguments)
{
    LOGI("===== start call method");
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        LOGI("===== prepare call method");
        LuaArgumentList argumentList;

        if (arguments != NULL) {
            jsize length = env->GetArrayLength(arguments);
            for (int i = 0; i < length; ++i) {
                jobject item = env->GetObjectArrayElement(arguments, i);
                LuaValue *value = convertJLuaValueToLuaValue(env, item);
                if (value != NULL) {
                    argumentList.push_back(value);
                }
            }
        }

        const char *methodNameStr = env -> GetStringUTFChars(methodName, NULL);
        LuaValue *retValue = context->callMethod(methodNameStr, argumentList);
        retObj = convertLuaValueToJLuaValue(env, retValue);
        retValue -> release();

        //释放参数内存
        for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    return retObj;
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    releaseNativeObject
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_releaseNativeObject
(JNIEnv *env, jclass obj, jint nativeObjectId)
{
    LuaObjectManager::SharedInstance() -> removeObject(nativeObjectId);
}
