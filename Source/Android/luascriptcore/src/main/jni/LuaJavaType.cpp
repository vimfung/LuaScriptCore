//
// Created by 冯鸿杰 on 16/9/28.
//

#include <stdint.h>
#include "LuaJavaType.h"

jclass LuaJavaType::fieldClass(JNIEnv *env)
{
    static jclass fieldClass = NULL;

    if (fieldClass == NULL)
    {
        jclass jTmpClass = env -> FindClass("java/lang/reflect/Field");
        fieldClass = (jclass)env -> NewGlobalRef(jTmpClass);
        env -> DeleteLocalRef(jTmpClass);
    }

    return fieldClass;
}

jclass LuaJavaType::methodClass(JNIEnv *env)
{
    static jclass methodClass = NULL;

    if (methodClass == NULL)
    {
        jclass jTmpClass = env -> FindClass("java/lang/reflect/Method");
        methodClass = (jclass)env -> NewGlobalRef(jTmpClass);
        env -> DeleteLocalRef(jTmpClass);
    }

    return methodClass;
}

jclass LuaJavaType::contextClass(JNIEnv *env)
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

jclass LuaJavaType::moduleClass(JNIEnv *env)
{
    static jclass jLuaModule = NULL;

    if (jLuaModule == NULL)
    {
        jclass jTmpClass = env -> FindClass("cn/vimfung/luascriptcore/LuaModule");
        jLuaModule = (jclass)env -> NewGlobalRef(jTmpClass);
        env -> DeleteLocalRef(jTmpClass);
    }

    return jLuaModule;
}

jclass LuaJavaType::luaValueClass(JNIEnv *env)
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

jclass LuaJavaType::luaValueTypeClass(JNIEnv *env)
{
    static jclass jLuaValueType = NULL;

    if (jLuaValueType == NULL)
    {
        jclass jLuaValueCls = env -> FindClass("cn/vimfung/luascriptcore/LuaValueType");
        jLuaValueType = (jclass)env -> NewGlobalRef(jLuaValueCls);
        env -> DeleteLocalRef(jLuaValueCls);
    }

    return jLuaValueType;
}

jclass LuaJavaType::stringClass(JNIEnv *env)
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

jclass LuaJavaType::shortClass(JNIEnv *env)
{
    static jclass jShortCls = NULL;

    if (jShortCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Short");
        jShortCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jShortCls;
}

jclass LuaJavaType::integerClass(JNIEnv *env)
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

jclass LuaJavaType::doubleClass(JNIEnv *env)
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

jclass LuaJavaType::floatClass(JNIEnv *env)
{
    static jclass jFloatCls = NULL;

    if (jFloatCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Float");
        jFloatCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jFloatCls;
}

jclass LuaJavaType::longClass(JNIEnv *env)
{
    static jclass jLongCls = NULL;

    if (jLongCls == NULL)
    {
        jclass tmpClass = env -> FindClass("java/lang/Long");
        jLongCls = (jclass)env -> NewGlobalRef(tmpClass);
        env -> DeleteLocalRef(tmpClass);
    }

    return jLongCls;
}

jclass LuaJavaType::booleanClass(JNIEnv *env)
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

jclass LuaJavaType::byteClass(JNIEnv *env)
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

jclass LuaJavaType::bytesClass(JNIEnv *env)
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

jclass LuaJavaType::byteArrayClass(JNIEnv *env)
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

jclass LuaJavaType::arrayListClass(JNIEnv *env)
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

jclass LuaJavaType::hashMapClass(JNIEnv *env)
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

jclass LuaJavaType::luaBaseObjectClass(JNIEnv *env)
{
    static jclass jLuaBaseObject = NULL;

    if (jLuaBaseObject == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/LuaBaseObject");
        jLuaBaseObject = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jLuaBaseObject;
}

jclass LuaJavaType::luaObjectClass(JNIEnv *env)
{
    static jclass jLuaObject = NULL;

    if (jLuaObject == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaObjectClass");
        jLuaObject = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jLuaObject;
}

jclass LuaJavaType::pointerClass(JNIEnv *env)
{
    static jclass jPointer = NULL;

    if (jPointer == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/LuaPointer");
        jPointer = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jPointer;
}

jclass LuaJavaType::functionClass(JNIEnv *env)
{
    static jclass jFunction = NULL;

    if (jFunction == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/LuaFunction");
        jFunction = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jFunction;
}

jclass LuaJavaType::tupleClass(JNIEnv *env)
{
    static jclass jTuple = NULL;

    if (jTuple == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/LuaTuple");
        jTuple = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jTuple;
}

jclass LuaJavaType::exportTypeManagerClass(JNIEnv *env)
{
    static jclass jExportTypeManagerCls = NULL;

    if (jExportTypeManagerCls == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/LuaExportTypeManager");
        jExportTypeManagerCls = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jExportTypeManagerCls;
}

jclass LuaJavaType::luaExportTypeClass(JNIEnv *env)
{
    static jclass jExportTypeCls = NULL;

    if (jExportTypeCls == NULL)
    {
        jclass jTempClass = env -> FindClass("cn/vimfung/luascriptcore/LuaExportType");
        jExportTypeCls = (jclass)env -> NewGlobalRef(jTempClass);
        env -> DeleteLocalRef(jTempClass);
    }

    return jExportTypeCls;
}