//
// Created by 冯鸿杰 on 16/9/30.
//

#include <stdint.h>
#include <LuaTuple.h>
#include "LuaJavaConverter.h"
#include "LuaJavaType.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaJavaObjectInstanceDescriptor.h"
#include "LuaObjectManager.h"
#include "LuaFunction.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaPointer.h"

LuaContext* LuaJavaConverter::convertToContextByJLuaContext(JNIEnv *env, jobject context)
{
    jfieldID nativeIdFieldId = env -> GetFieldID(LuaJavaType::contextClass(env), "_nativeId", "I");
    jint nativeId = env -> GetIntField(context, nativeIdFieldId);

    return (LuaContext *)LuaObjectManager::SharedInstance() -> getObject(nativeId);
}

LuaValue* LuaJavaConverter::convertToLuaValueByJObject(JNIEnv *env, LuaContext *context, jobject object)
{
    LuaValue *value = NULL;

    if (env -> IsInstanceOf(object, LuaJavaType::stringClass(env)) == JNI_TRUE)
    {
        //String类型
        jstring str = (jstring) object;
        const char *cstr = env -> GetStringUTFChars(str, NULL);
        std::string valueStr = cstr;
        value = new LuaValue(valueStr);
        env -> ReleaseStringUTFChars(str, cstr);
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::integerClass(env)) == JNI_TRUE)
    {
        //Integer类型
        jmethodID intValueMethodId = env -> GetMethodID(LuaJavaType::integerClass(env), "intValue", "()I");
        value = new LuaValue((long)env -> CallIntMethod(object, intValueMethodId));
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::doubleClass(env)) == JNI_TRUE)
    {
        //Double类型
        jmethodID  doubleValueMethodId = env -> GetMethodID(LuaJavaType::doubleClass(env), "doubleValue", "()D");
        value = new LuaValue(env -> CallDoubleMethod(object, doubleValueMethodId));
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::booleanClass(env)) == JNI_TRUE)
    {
        //Boolean类型
        jmethodID  boolValueMethodId = env -> GetMethodID(LuaJavaType::booleanClass(env), "booleanValue", "()Z");
        value = new LuaValue((bool)env -> CallBooleanMethod(object, boolValueMethodId));
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::bytesClass(env)) == JNI_TRUE)
    {
        //byte数组
        jbyteArray byteArr = (jbyteArray)object;
        jsize len = env -> GetArrayLength(byteArr);
        jbyte bytes[len];
        env -> GetByteArrayRegion(byteArr,0, len, bytes);

        value = new LuaValue((const char *)bytes, (size_t)len);
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::byteArrayClass(env)) == JNI_TRUE)
    {
        //byte数组
        jmethodID  byteValueMethodId = env -> GetMethodID(LuaJavaType::byteClass(env), "byteValue", "()B");

        jobjectArray byteArr = (jobjectArray)object;
        jsize len = env -> GetArrayLength(byteArr);

        jbyte bytes[len];
        for (int i = 0; i < len; ++i)
        {
            jobject byteItem = env -> GetObjectArrayElement(byteArr, i);
            bytes[i] = env -> CallByteMethod(byteItem, byteValueMethodId);
            env -> DeleteLocalRef(byteItem);
        }

        value = new LuaValue((const char *)bytes, (size_t)len);
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::arrayListClass(env)) == JNI_TRUE)
    {
        //ArrayList
        static jclass jArrayListClass = LuaJavaType::arrayListClass(env);
        static jmethodID getMethodId = env -> GetMethodID(jArrayListClass, "get", "(I)Ljava/lang/Object;");
        static jmethodID sizeMethodId = env -> GetMethodID(jArrayListClass, "size", "()I");

        LuaValueList list;
        jint len = env -> CallIntMethod(object, sizeMethodId);
        for (int i = 0; i < len; ++i)
        {
            jobject item = env -> CallObjectMethod(object, getMethodId, i);
            LuaValue *valueItem = LuaJavaConverter::convertToLuaValueByJObject(env, context, item);
            env -> DeleteLocalRef(item);
        }

        value = new LuaValue(list);
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::hashMapClass(env)) == JNI_TRUE)
    {
        //HashMap
        static jclass jHashMapClass = LuaJavaType::hashMapClass(env);
        static jmethodID getMethodId = env -> GetMethodID(jHashMapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        static jmethodID sizeMethodId = env -> GetMethodID(jHashMapClass, "size", "()I");
        static jmethodID keySetMethodId = env -> GetMethodID(jHashMapClass, "keySet", "()Ljava/util/Set;");

        static jclass jSetClass = (jclass)env -> NewGlobalRef(env -> FindClass("java/util/Set"));
        static jmethodID toArrayMethodId = env -> GetMethodID(jSetClass, "toArray", "()[Ljava/lang/Object;");

        LuaValueMap map;
        jint len = env -> CallIntMethod(object, sizeMethodId);

        jobject keySet= env -> CallObjectMethod(object, keySetMethodId);
        jobjectArray keys = (jobjectArray)env -> CallObjectMethod(keySet, toArrayMethodId);

        for (int i = 0; i < len; ++i)
        {
            jobject key = env -> GetObjectArrayElement(keys, i);
            jobject item = env -> CallObjectMethod(object, getMethodId, key);

            const char *keyStr = env -> GetStringUTFChars((jstring)key, NULL);
            LuaValue *valueItem = LuaJavaConverter::convertToLuaValueByJObject(env, context, item);
            map[keyStr] = valueItem;
            env -> ReleaseStringUTFChars((jstring)key, keyStr);

            env -> DeleteLocalRef(item);
            env -> DeleteLocalRef(key);
        }

        value = new LuaValue(map);

        env -> DeleteLocalRef(keys);
        env -> DeleteLocalRef(keySet);
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::luaValueClass(env)) == JNI_TRUE)
    {
        //LuaValue类型
        value = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, object);
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::pointerClass(env)) == JNI_TRUE)
    {
        //LuaPointer类型
        //先查找LuaPointer是否有对应的本地对象
        jfieldID nativeIdFieldId = env -> GetFieldID(LuaJavaType::pointerClass(env), "_nativeId", "I");
        jint nativeId = env -> GetIntField(object, nativeIdFieldId);
        LuaPointer *pointer = (LuaPointer *)LuaObjectManager::SharedInstance() -> getObject(nativeId);
        if (pointer != NULL)
        {
            value = new LuaValue(pointer);
        }
        else
        {
            //没找到相关的Pointer则使用nil代替
            value = new LuaValue();
        }
    }
    else if (env -> IsInstanceOf(object, LuaJavaType::functionClass(env)) == JNI_TRUE)
    {
        //LuaFunction类型
        jfieldID nativeIdFieldId = env -> GetFieldID(LuaJavaType::functionClass(env), "_nativeId", "I");
        jint nativeId = env -> GetIntField(object, nativeIdFieldId);
        LuaFunction *function = (LuaFunction *)LuaObjectManager::SharedInstance() -> getObject(nativeId);
        if (function != NULL)
        {
            value = new LuaValue(function);
        }
        else
        {
            //没找到相关的Function则使用nil代替
            value = new LuaValue();
        }

    }
    else if (env -> IsInstanceOf(object, LuaJavaType::tupleClass(env)) == JNI_TRUE)
    {
        //LuaTuple类型
        jmethodID countMethodId = env -> GetMethodID(LuaJavaType::tupleClass(env), "count", "()I");
        jmethodID returnValueMethodId = env -> GetMethodID(LuaJavaType::tupleClass(env), "getReturnValueByIndex", "(I)Ljava/lang/Object;");
        jint count = env -> CallIntMethod(object, countMethodId);

        LuaTuple *tuple = new LuaTuple();
        for (int i = 0; i < count; ++i)
        {
            jobject returnValue = env -> CallObjectMethod(object, returnValueMethodId, i);
            LuaValue *value = LuaJavaConverter::convertToLuaValueByJObject(env, context, returnValue);
            if (value != NULL)
            {
                tuple -> addReturnValue(value);
                value -> release();
            }
            env -> DeleteLocalRef(returnValue);
        }

        value = new LuaValue(tuple);
    }
    else if (env -> IsSameObject(object, NULL) != JNI_TRUE)
    {
        //对象类型
        //查找对象是否存在ObjectDescriptor
        bool needRelease = false;
        LuaObjectDescriptor *objDesc = LuaJavaEnv::getAssociateInstanceRef(env, object);
        if (objDesc == NULL)
        {
            //不存在则创建对象
            if (env -> IsInstanceOf(object, LuaJavaType::luaObjectClass(env)))
            {
                //为LuaObjectClass
                LuaJavaObjectClass *objectClass = LuaJavaEnv::getObjectClassByInstance(env, object, context);
                objDesc = new LuaJavaObjectInstanceDescriptor(env, object, objectClass);
            }
            else
            {
                objDesc = new LuaJavaObjectDescriptor(env, object);
            }
            needRelease = true;
        }

        value = new LuaValue(objDesc);

        if (needRelease)
        {
            objDesc->release();
        }
    }

    return value;
}

LuaValue* LuaJavaConverter::convertToLuaValueByJLuaValue(JNIEnv *env, LuaContext *context, jobject value)
{
    //构造调用参数
    static jclass jLuaValueTypeClass = LuaJavaType::luaValueTypeClass(env);
    static jmethodID typeValueMethodId = env -> GetMethodID(jLuaValueTypeClass, "value", "()I");

    static jclass jLuaValueClass = LuaJavaType::luaValueClass(env);
    static jmethodID typeMethodId = env -> GetMethodID(jLuaValueClass, "valueType", "()Lcn/vimfung/luascriptcore/LuaValueType;");
    static jmethodID toIntMethodId = env -> GetMethodID(jLuaValueClass, "toInteger", "()I");
    static jmethodID toNumMethodId = env -> GetMethodID(jLuaValueClass, "toDouble", "()D");
    static jmethodID toBoolMethodId = env -> GetMethodID(jLuaValueClass, "toBoolean", "()Z");
    static jmethodID toStrMethodId = env -> GetMethodID(jLuaValueClass, "toString", "()Ljava/lang/String;");
    static jmethodID toByteArrMethodId = env -> GetMethodID(jLuaValueClass, "toByteArray", "()[B");
    static jmethodID toListMethodId = env -> GetMethodID(jLuaValueClass, "toArrayList", "()Ljava/util/ArrayList;");
    static jmethodID toMapMethodId = env -> GetMethodID(jLuaValueClass, "toHashMap", "()Ljava/util/HashMap;");
    static jmethodID toPointerId = env -> GetMethodID(jLuaValueClass, "toPointer", "()Lcn/vimfung/luascriptcore/LuaPointer;");
    static jmethodID toFunctionId = env -> GetMethodID(jLuaValueClass, "toFunction", "()Lcn/vimfung/luascriptcore/LuaFunction;");
    static jmethodID toObjectId = env -> GetMethodID(jLuaValueClass, "toObject", "()Ljava/lang/Object;");
    static jmethodID toTupleId = env -> GetMethodID(jLuaValueClass, "toTuple", "()Lcn/vimfung/luascriptcore/LuaTuple;");

    jobject itemType = env -> CallObjectMethod(value, typeMethodId);
    jint valueType = env -> CallIntMethod(itemType, typeValueMethodId);
    env -> DeleteLocalRef(itemType);

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
            env -> DeleteLocalRef(strValue);
            break;
        }
        case LuaValueTypeData:
        {
            jbyteArray byteArr = (jbyteArray)env -> CallObjectMethod(value, toByteArrMethodId);
            jsize len = env -> GetArrayLength(byteArr);

            jbyte buffer[len];
            env -> GetByteArrayRegion(byteArr, 0, len, buffer);

            retValue = LuaValue::DataValue((const char *)buffer, (size_t)len);

            env -> DeleteLocalRef(byteArr);

            break;
        }
        case LuaValueTypeArray:
        {
            static jclass jArrayListClass = LuaJavaType::arrayListClass(env);
            static jmethodID getMethodId = env -> GetMethodID(jArrayListClass, "get", "(I)Ljava/lang/Object;");
            static jmethodID sizeMethodId = env -> GetMethodID(jArrayListClass, "size", "()I");

            LuaValueList list;
            jobject arrayList = env -> CallObjectMethod(value, toListMethodId);
            jint len = env -> CallIntMethod(arrayList, sizeMethodId);
            for (int i = 0; i < len; ++i)
            {
                jobject item = env -> CallObjectMethod(arrayList, getMethodId, i);
                LuaValue *valueItem = LuaJavaConverter::convertToLuaValueByJObject(env, context, item);
                env -> DeleteLocalRef(item);
            }

            retValue = LuaValue::ArrayValue(list);

            env -> DeleteLocalRef(arrayList);
            break;
        }
        case LuaValueTypeMap:
        {
            static jclass jHashMapClass = LuaJavaType::hashMapClass(env);
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
                LuaValue *valueItem = LuaJavaConverter::convertToLuaValueByJObject(env, context, item);
                map[keyStr] = valueItem;
                env -> ReleaseStringUTFChars((jstring)key, keyStr);

                env -> DeleteLocalRef(item);
                env -> DeleteLocalRef(key);
            }

            retValue = LuaValue::DictonaryValue(map);

            env -> DeleteLocalRef(keys);
            env -> DeleteLocalRef(keySet);
            env -> DeleteLocalRef(hashMap);

            break;
        }
        case LuaValueTypePtr:
        {
            jobject jPointer = env -> CallObjectMethod(value, toPointerId);

            //先查找是否有对应的本地LuaPointer
            jfieldID nativeIdFieldId = env -> GetFieldID(LuaJavaType::pointerClass(env), "_nativeId", "I");
            jint nativeId = env -> GetIntField(jPointer, nativeIdFieldId);

            LuaPointer *pointer = (LuaPointer *)LuaObjectManager::SharedInstance() -> getObject(nativeId);
            if (pointer != NULL)
            {
                retValue = new LuaValue(pointer);
            }
            else
            {
                retValue = new LuaValue();
            }

            env -> DeleteLocalRef(jPointer);

            break;
        }
        case LuaValueTypeFunction:
        {
            jobject jFunction = env -> CallObjectMethod(value, toFunctionId);

            //先查找是否有对应的本地LuaFunction
            jfieldID nativeIdFieldId = env -> GetFieldID(LuaJavaType::functionClass(env), "_nativeId", "I");
            jint nativeId = env -> GetIntField(jFunction, nativeIdFieldId);

            LuaFunction *function = (LuaFunction *)LuaObjectManager::SharedInstance() -> getObject(nativeId);
            if (function != NULL)
            {
                retValue = new LuaValue(function);
            }
            else
            {
                retValue = new LuaValue();
            }

            env -> DeleteLocalRef(jFunction);

            break;
        }
        case LuaValueTypeTuple:
        {
            //元组
            jobject jTuple = env -> CallObjectMethod(value, toTupleId);

            jmethodID countMethodId = env -> GetMethodID(LuaJavaType::tupleClass(env), "count", "()I");
            jmethodID returnValueMethodId = env -> GetMethodID(LuaJavaType::tupleClass(env), "getReturnValueByIndex", "(I)Ljava/lang/Object;");
            jint count = env -> CallIntMethod(jTuple, countMethodId);

            LuaTuple *tuple = new LuaTuple();
            for (int i = 0; i < count; ++i)
            {
                jobject returnValue = env -> CallObjectMethod(jTuple, returnValueMethodId, i);
                LuaValue *item = LuaJavaConverter::convertToLuaValueByJObject(env, context, returnValue);
                if (value != NULL)
                {
                    tuple -> addReturnValue(item);
                    item -> release();
                }

                env -> DeleteLocalRef(returnValue);
            }

            retValue = new LuaValue(tuple);

            env -> DeleteLocalRef(jTuple);

            break;
        }
        case LuaValueTypeObject:
        {
            jobject obj = env -> CallObjectMethod(value, toObjectId);
            if (env -> IsSameObject(obj, NULL) != JNI_TRUE)
            {
                bool needRelease = false;
                LuaObjectDescriptor *objDesc = LuaJavaEnv::getAssociateInstanceRef(env, obj);
                if (objDesc == NULL)
                {
                    //不存在则创建对象
                    if (env -> IsInstanceOf(obj, LuaJavaType::luaObjectClass(env)))
                    {
                        //为LuaObjectClass
                        LuaJavaObjectClass *objectClass = LuaJavaEnv::getObjectClassByInstance(env, obj, context);
                        objDesc = new LuaJavaObjectInstanceDescriptor(env, obj, objectClass);
                    }
                    else
                    {
                        objDesc = new LuaJavaObjectDescriptor(env, obj);
                    }

                    needRelease = true;
                }

                retValue = new LuaValue(objDesc);

                if (needRelease)
                {
                    objDesc->release();
                }

                env -> DeleteLocalRef(obj);
            }
            break;
        }
        default:
            break;
    }

    return retValue;
}

jobject LuaJavaConverter::convertToJavaObjectByLuaValue(JNIEnv *env, LuaContext *context, LuaValue *luaValue)
{
    jobject retObj = NULL;
    if (luaValue != NULL)
    {
        switch (luaValue -> getType())
        {
            case LuaValueTypeNumber:
            {
                static jmethodID initMethodId = env -> GetMethodID(LuaJavaType::doubleClass(env), "<init>", "(D)V");
                retObj = env -> NewObject(LuaJavaType::doubleClass(env), initMethodId, luaValue -> toNumber());

                break;
            }
            case LuaValueTypeBoolean:
            {
                static jmethodID initMethodId = env -> GetMethodID(LuaJavaType::booleanClass(env), "<init>", "(Z)V");
                retObj = env -> NewObject(LuaJavaType::booleanClass(env), initMethodId, luaValue -> toBoolean());

                break;
            }
            case LuaValueTypeString:
            {
                retObj = env->NewStringUTF(luaValue->toString().c_str());
                break;
            }
            case LuaValueTypeData:
            {
                jsize size = (jsize)luaValue -> getDataLength();
                jbyteArray byteArray = env -> NewByteArray(size);
                env -> SetByteArrayRegion(byteArray, 0, size, (const jbyte *)luaValue -> toData());

                retObj = byteArray;

                break;
            }
            case LuaValueTypeArray:
            {
                LuaValueList *list = luaValue -> toArray();
                if (list != NULL)
                {
                    static jclass jArrayListClass = LuaJavaType::arrayListClass(env);
                    static jmethodID initMethodId = env->GetMethodID(jArrayListClass, "<init>", "()V");
                    static jmethodID addMethodId = env->GetMethodID(jArrayListClass, "add", "(Ljava/lang/Object;)Z");

                    retObj = env->NewObject(jArrayListClass, initMethodId);
                    for (LuaValueList::iterator i = list->begin(); i != list->end(); ++i)
                    {
                        LuaValue *item = *i;
                        jobject itemObj = LuaJavaConverter::convertToJavaObjectByLuaValue(env, context, item);
                        env -> CallBooleanMethod(retObj, addMethodId, itemObj);
                        if (itemObj != NULL)
                        {
                            env -> DeleteLocalRef(itemObj);
                        }
                    }
                }
                break;
            }
            case LuaValueTypeMap:
            {
                LuaValueMap *map = luaValue -> toMap();
                if (map != NULL)
                {
                    static jclass jHashMapClass = LuaJavaType::hashMapClass(env);
                    static jmethodID initMethodId = env -> GetMethodID(jHashMapClass, "<init>", "()V");
                    static jmethodID putMethodId = env -> GetMethodID(jHashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

                    retObj = env -> NewObject(jHashMapClass, initMethodId);
                    for (LuaValueMap::iterator i = map -> begin(); i != map -> end() ; ++i)
                    {
                        std::string key = i -> first;
                        LuaValue *item = i -> second;

                        jstring keyStr = env -> NewStringUTF(key.c_str());
                        jobject itemObj = LuaJavaConverter::convertToJavaObjectByLuaValue(env, context, item);
                        if (keyStr != NULL && itemObj != NULL)
                        {
                            jobject tmpObj = env -> CallObjectMethod(retObj, putMethodId, keyStr, itemObj);
                            env -> DeleteLocalRef(tmpObj);
                        }

                        env -> DeleteLocalRef(keyStr);
                        env -> DeleteLocalRef(itemObj);
                    }
                }
                break;
            }
            case LuaValueTypePtr:
            {
                LuaPointer *pointer = luaValue -> toPointer();
                if (pointer != NULL)
                {
                    int nativeId = LuaObjectManager::SharedInstance() -> putObject(pointer);
                    //创建Java层的LuaPointer对象
                    jmethodID initMethodId = env -> GetMethodID(LuaJavaType::pointerClass(env), "<init>", "(I)V");
                    retObj = env -> NewObject(LuaJavaType::pointerClass(env), initMethodId, nativeId);
                }
                break;
            }
            case LuaValueTypeFunction:
            {
                LuaFunction *function = luaValue -> toFunction();
                if (function != NULL)
                {
                    int nativeId = LuaObjectManager::SharedInstance() -> putObject(function);
                    jmethodID initMethodId = env -> GetMethodID(LuaJavaType::functionClass(env), "<init>", "(ILcn/vimfung/luascriptcore/LuaContext;)V");
                    jobject jcontext = LuaJavaEnv::getJavaLuaContext(env, context);
                    retObj = env -> NewObject(LuaJavaType::functionClass(env), initMethodId, nativeId, jcontext);
                }
                break;
            }
            case LuaValueTypeTuple:
            {
                //元组
                LuaTuple *tuple = luaValue -> toTuple();
                if (tuple != NULL)
                {
                    jmethodID initMethodId = env -> GetMethodID(LuaJavaType::tupleClass(env), "<init>", "()V");
                    jmethodID addReturnValueMethodId = env -> GetMethodID(LuaJavaType::tupleClass(env), "addReturnValue", "(Ljava/lang/Object;)V");

                    retObj = env -> NewObject(LuaJavaType::tupleClass(env), initMethodId);

                    //填充数据
                    for (int i = 0; i < tuple->count(); ++i)
                    {
                        LuaValue *retValue = tuple -> getReturnValueByIndex(i);
                        jobject jRetValue = LuaJavaConverter::convertToJavaObjectByLuaValue(env, context, retValue);
                        if (jRetValue != NULL)
                        {
                            env -> CallVoidMethod(retObj, addReturnValueMethodId, jRetValue);
                        }
                        env -> DeleteLocalRef(jRetValue);
                    }
                }
                break;
            }
            case LuaValueTypeObject:
            {
                LuaObjectDescriptor *objDesc = luaValue -> toObject();
                if (dynamic_cast<LuaJavaObjectDescriptor *>(objDesc) != NULL
                        || dynamic_cast<LuaJavaObjectInstanceDescriptor *>(objDesc) != NULL)
                {
                    //如果为LuaJavaObjectDescriptor则转换为jobject类型
                    retObj = env -> NewLocalRef((jobject)objDesc -> getObject());
                }
                break;
            }
            default:
                break;
        }
    }

    return retObj;
}

jobject LuaJavaConverter::convertToJavaLuaValueByLuaValue(JNIEnv *env, LuaContext *context, LuaValue *luaValue)
{
    jobject retObj = NULL;
    if (luaValue != NULL)
    {
        static jclass jLuaValue = LuaJavaType::luaValueClass(env);
        static jmethodID jNilInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(I)V");
        jmethodID initMethodId = jNilInitMethodId;

        switch (luaValue->getType())
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
            case LuaValueTypePtr:
            {
                static jmethodID pointerInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILcn/vimfung/luascriptcore/LuaPointer;)V");
                initMethodId = pointerInitMethodId;
                break;
            }
            case LuaValueTypeFunction:
            {
                static jmethodID functionInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILcn/vimfung/luascriptcore/LuaFunction;)V");
                initMethodId = functionInitMethodId;
                break;
            }
            case LuaValueTypeTuple:
            {
                static jmethodID tupleInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILcn/vimfung/luascriptcore/LuaTuple;)V");
                initMethodId = tupleInitMethodId;
                break;
            }
            case LuaValueTypeObject:
            {
                static jmethodID objectInitMethodId = env -> GetMethodID(jLuaValue, "<init>", "(ILjava/lang/Object;)V");
                initMethodId = objectInitMethodId;
                break;
            }
            default:
                break;
        }

//        int objectId = LuaObjectManager::SharedInstance() -> putObject(luaValue);

        if (luaValue -> getType() == LuaValueTypeNil)
        {
            retObj = env -> NewObject(jLuaValue, initMethodId, luaValue -> objectId());
        }
        else
        {
            jobject jObj = LuaJavaConverter::convertToJavaObjectByLuaValue(env, context, luaValue);
            retObj = env -> NewObject(jLuaValue, initMethodId, luaValue -> objectId(), jObj);
            env -> DeleteLocalRef(jObj);
        }
    }

    return retObj;
}
