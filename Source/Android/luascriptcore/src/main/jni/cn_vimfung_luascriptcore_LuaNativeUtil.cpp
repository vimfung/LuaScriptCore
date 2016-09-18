//
// Created by vimfung on 16/8/29.
//
#include <stddef.h>
#include "cn_vimfung_luascriptcore_LuaNativeUtil.h"

#include "LuaContext.h"
#include "LuaObjectManager.h"
#include "LuaDefine.h"

using namespace cn::vimfung::luascriptcore;

//Java层的LuaContext引用
std::map<jint, jobject> _jcontextRefs;

static bool _attatedT = false;
static JavaVM *_javaVM;

LuaValue* convertJLuaValueToLuaValue (JNIEnv *env, jobject value);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    _javaVM = vm;
    return JNI_VERSION_1_4;
}

static JNIEnv *GetEnv()
{
    int status;

    JNIEnv *envnow = NULL;
    status = _javaVM -> GetEnv((void **)&envnow, JNI_VERSION_1_4);

    if(status < 0)
    {
        status = _javaVM -> AttachCurrentThread(&envnow, NULL);
        if(status < 0)
        {
            return NULL;
        }
        _attatedT = true;
    }

    return envnow;
}

static void DetachCurrent()
{
    if(_attatedT)
    {
        _javaVM -> DetachCurrentThread();
    }
}

/**
 * 获取Java层中的LuaContext类
 */
jclass getJLuaContextClass(JNIEnv *env)
{
    static jclass jLuaContext = NULL;

    if (jLuaContext == NULL)
    {
        jclass jLuaContextCls = env -> FindClass("cn/vimfung/luascriptcore/LuaContext");
        jLuaContext = (jclass)env -> NewGlobalRef(jLuaContextCls);
        env -> DeleteLocalRef(jLuaContextCls);
    }

    return jLuaContext;
}

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
 * 获取Java层String类型
 */
jclass getJStringClass(JNIEnv *env)
{
    static jclass jStringCls = NULL;

    if (jStringCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/String");
        jStringCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jStringCls;
}

/**
 * 获取Java层Integer类型
 */
jclass getJIntegerClass(JNIEnv *env)
{
    static jclass jIntegerCls = NULL;

    if (jIntegerCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Integer");
        jIntegerCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jIntegerCls;
}

/**
 * 获取Java层Double类型
 */
jclass getJDoubleClass(JNIEnv *env)
{
    static jclass jDoubleCls = NULL;

    if (jDoubleCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Double");
        jDoubleCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jDoubleCls;
}

/**
 * 获取Java层Boolean类型
 */
jclass getJBooleanClass(JNIEnv *env)
{
    static jclass jBooleanCls = NULL;

    if (jBooleanCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Boolean");
        jBooleanCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jBooleanCls;
}

/**
 * 获取Java层Byte类型
 */
jclass getJByteClass(JNIEnv *env)
{
    static jclass jByteCls = NULL;

    if (jByteCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Byte");
        jByteCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jByteCls;
}

/**
 * 获取Java层byte数组类型
 */
jclass getJBytesClass(JNIEnv *env)
{
    static jclass jByteArrayCls = NULL;

    if (jByteArrayCls == NULL)
    {
        jclass tmpClass = env -> FindClass("[B");
        jByteArrayCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jByteArrayCls;
}

/**
 * 获取Java层Byte数组类型
 */
jclass getJBytesArrayClass(JNIEnv *env)
{
    static jclass jByteArrayCls = NULL;

    if (jByteArrayCls == NULL)
    {
        jclass tmpClass = env -> FindClass("[Ljava/lang/Byte;");
        jByteArrayCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jByteArrayCls;
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

/**
 * 转换Java层的对象LuaValue对象
 */
LuaValue* convertJObjectToLuaValue(JNIEnv *env, jobject obj)
{
    LuaValue *value = NULL;

    if (env -> IsInstanceOf(obj, getJStringClass(env)) == JNI_TRUE)
    {
        //String类型
        jstring str = (jstring) obj;
        const char *cstr = env -> GetStringUTFChars(str, NULL);
        std::string valueStr = cstr;
        value = new LuaValue(valueStr);
        env -> ReleaseStringUTFChars(str, cstr);
    }
    else if (env -> IsInstanceOf(obj, getJIntegerClass(env)) == JNI_TRUE)
    {
        //Integer类型
        jmethodID intValueMethodId = env -> GetMethodID(getJIntegerClass(env), "intValue", "()I");
        value = new LuaValue((long)env -> CallIntMethod(obj, intValueMethodId));
    }
    else if (env -> IsInstanceOf(obj, getJDoubleClass(env)) == JNI_TRUE)
    {
        //Double类型
        jmethodID  doubleValueMethodId = env -> GetMethodID(getJDoubleClass(env), "doubleValue", "()D");
        value = new LuaValue(env -> CallDoubleMethod(obj, doubleValueMethodId));
    }
    else if (env -> IsInstanceOf(obj, getJBooleanClass(env)) == JNI_TRUE)
    {
        //Boolean类型
        jmethodID  boolValueMethodId = env -> GetMethodID(getJDoubleClass(env), "booleanValue", "()Z");
        value = new LuaValue((bool)env -> CallBooleanMethod(obj, boolValueMethodId));
    }
    else if (env -> IsInstanceOf(obj, getJBytesClass(env)) == JNI_TRUE)
    {
        //byte数组
        jbyteArray byteArr = (jbyteArray)obj;
        jsize len = env -> GetArrayLength(byteArr);
        jbyte bytes[len];
        env -> GetByteArrayRegion(byteArr,0, len, bytes);

        value = new LuaValue((const char *)bytes, (size_t)len);
    }
    else if (env -> IsInstanceOf(obj, getJBytesArrayClass(env)) == JNI_TRUE)
    {
        //byte数组
        jmethodID  byteValueMethodId = env -> GetMethodID(getJByteClass(env), "byteValue", "()B");

        jobjectArray byteArr = (jobjectArray)obj;
        jsize len = env -> GetArrayLength(byteArr);

        jbyte bytes[len];
        for (int i = 0; i < len; ++i)
        {
            jobject byteItem = env -> GetObjectArrayElement(byteArr, i);
            bytes[i] = env -> CallByteMethod(byteItem, byteValueMethodId);
        }

        value = new LuaValue((const char *)bytes, (size_t)len);
    }
    else if (env -> IsInstanceOf(obj, getJLuaValueClass(env)) == JNI_TRUE)
    {
        //LuaValue类型
        value = convertJLuaValueToLuaValue(env, obj);
    }
    else
    {
        value = new LuaValue();
    }

    return value;
}

LuaValue* convertJLuaValueToLuaValue (JNIEnv *env, jobject value)
{
    //构造调用参数
    static jclass jLuaValueType = (jclass)env -> NewGlobalRef(env -> FindClass("cn/vimfung/luascriptcore/LuaValueType"));
    static jmethodID typeValueMethodId = env -> GetMethodID(jLuaValueType, "value", "()I");

    static jclass jLuaValueClass = getJLuaValueClass(env);
    static jmethodID typeMethodId = env -> GetMethodID(jLuaValueClass, "valueType", "()Lcn/vimfung/luascriptcore/LuaValueType;");
    static jmethodID toIntMethodId = env -> GetMethodID(jLuaValueClass, "toInteger", "()I");
    static jmethodID toNumMethodId = env -> GetMethodID(jLuaValueClass, "toDouble", "()D");
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
            env -> ReleaseStringUTFChars(strValue, charStr);
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
            static jmethodID getMethodId = env -> GetMethodID(jArrayListClass, "get", "(I)Ljava/lang/Object;");
            static jmethodID sizeMethodId = env -> GetMethodID(jArrayListClass, "size", "()I");

            LuaValueList list;
            jobject arrayList = env -> CallObjectMethod(value, toListMethodId);
            jint len = env -> CallIntMethod(arrayList, sizeMethodId);
            for (int i = 0; i < len; ++i)
            {
                jobject item = env -> CallObjectMethod(arrayList, getMethodId, i);
                LuaValue *valueItem = convertJObjectToLuaValue(env, item);
            }

            retValue = LuaValue::ArrayValue(list);
            break;
        }
        case LuaValueTypeMap:
        {
            static jclass jHashMapClass = getJHashMapClass(env);
            static jmethodID getMethodId = env -> GetMethodID(jHashMapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
            static jmethodID sizeMethodId = env -> GetMethodID(jHashMapClass, "size", "()I");
            static jmethodID keySetMethodId = env -> GetMethodID(jHashMapClass, "keySet", "()Ljava/util/Set;");

            static jclass jSetClass = (jclass)env -> NewGlobalRef(env -> FindClass("java/util/Set"));
            static jmethodID toArrayMethodId = env -> GetMethodID(jSetClass, "toArray", "()[Ljava/lang/Object;");

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
                LuaValue *valueItem = convertJObjectToLuaValue(env, item);
                map[keyStr] = valueItem;
                env -> ReleaseStringUTFChars((jstring)key, keyStr);
            }

            retValue = LuaValue::DictonaryValue(map);

            break;
        }
        default:
            break;
    }

    return retValue;
}

/**
 * Lua方法处理器
 *
 * @param arguments 方法参数
 *
 * @returns 返回值
 */
LuaValue* luaMethodHandler (LuaContext *context, std::string methodName, LuaArgumentList arguments)
{
    std::map<jint, jobject>::iterator it =  _jcontextRefs.find(context->objectId());
    if (it != _jcontextRefs.end())
    {
        jobject jcontext = it -> second;

        JNIEnv *env = GetEnv();

        if (env -> IsSameObject(jcontext, NULL) != JNI_TRUE)
        {
            static jclass contenxtClass = getJLuaContextClass(env);
            static jmethodID invokeMethodID = env -> GetMethodID(contenxtClass, "methodInvoke", "(Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
            static jclass luaValueClass = getJLuaValueClass(env);

            jstring jMethodName = env -> NewStringUTF(methodName.c_str());

            //参数
            jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
            int index = 0;
            for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
            {
                LuaValue *argument = *it;
                jobject jArgument = convertLuaValueToJLuaValue(env, argument);
                env -> SetObjectArrayElement(argumentArr, index, jArgument);
                index++;
            }

            jobject result = env -> CallObjectMethod(jcontext, invokeMethodID, jMethodName, argumentArr);
            LuaValue *retValue = convertJLuaValueToLuaValue(env, result);

            DetachCurrent();

            return retValue;
        }
        else
        {
            //移除对象引用
            env -> DeleteWeakGlobalRef(jcontext);
            _jcontextRefs.erase(it);

            DetachCurrent();
        }
    }

    return NULL;
}

/*
 * Class:     cn_vimfung_luascriptcore_LuaNativeUtil
 * Method:    createContext
 * Signature: ()Lcn/vimfung/luascriptcore/LuaContext;
 */
JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_createContext
        (JNIEnv *env, jclass obj)
{
    static jclass contextClass = getJLuaContextClass(env);
    static jmethodID initMethodId = env -> GetMethodID(contextClass, "<init>", "(I)V");

    LuaContext *context = new LuaContext();
    int nativeId = LuaObjectManager::SharedInstance() -> putObject(context);
    context -> release();

    jobject jcontext = env -> NewObject(contextClass, initMethodId, nativeId);

    _jcontextRefs[context -> objectId()] = env -> NewWeakGlobalRef(jcontext);
    return jcontext;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_addSearchPath
        (JNIEnv *env, jclass thiz, jint nativeContextId, jstring path)
{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char *pathStr = env -> GetStringUTFChars(path, NULL);
        context -> addSearchPath(pathStr);
        env -> ReleaseStringUTFChars(path, pathStr);
    }
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
        env -> ReleaseStringUTFChars(script, scriptText);
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

        LuaValue *value = context->evalScriptFromFile(scriptPath);
        retObj = convertLuaValueToJLuaValue(env, value);
        value -> release();
        env -> ReleaseStringUTFChars(path, scriptPath);
    }

    return retObj;
}

JNIEXPORT jobject JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_callMethod
        (JNIEnv *env, jclass obj, jint nativeContextId, jstring methodName, jobjectArray arguments)
{
    jobject retObj = NULL;

    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
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
        env -> ReleaseStringUTFChars(methodName, methodNameStr);

        //释放参数内存
        for (LuaArgumentList::iterator it = argumentList.begin(); it != argumentList.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    return retObj;
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_registerMethod
        (JNIEnv *env, jclass thiz, jint nativeContextId, jstring methodName)
{
    LuaContext *context = (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeContextId);
    if (context != NULL)
    {
        const char *methodNameStr = env -> GetStringUTFChars(methodName, NULL);
        context -> registerMethod(methodNameStr, luaMethodHandler);
        env -> ReleaseStringUTFChars(methodName, methodNameStr);
    }
}

JNIEXPORT void JNICALL Java_cn_vimfung_luascriptcore_LuaNativeUtil_releaseNativeObject
(JNIEnv *env, jclass obj, jint nativeObjectId)
{
    std::map<jint, jobject>::iterator it =  _jcontextRefs.find(nativeObjectId);
    if (it != _jcontextRefs.end())
    {
        //为LuaContext对象,解除对象引用
        env -> DeleteWeakGlobalRef(it -> second);
        _jcontextRefs.erase(it);
    }

    LuaObjectManager::SharedInstance() -> removeObject(nativeObjectId);
}
