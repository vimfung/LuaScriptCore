//
// Created by 冯鸿杰 on 17/3/27.
//

#include "StringUtils.h"
#include "LuaJavaExportClassProxy.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaEnv.h"

using namespace cn::vimfung::luascriptcore::modules::oo;

LuaJavaExportClassProxy::LuaJavaExportClassProxy(const std::string &className)
    : LuaExportClassProxy()
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    std::string clsName = StringUtils::replace(className, ".", "/");
    jclass cls = env -> FindClass(clsName.c_str());
    if (cls != NULL)
    {
        _classDescriptor = new LuaJavaObjectDescriptor(env, cls);
    }

    LuaJavaEnv::resetEnv(env);
}

LuaJavaExportClassProxy::~LuaJavaExportClassProxy()
{
    if (_classDescriptor != NULL)
    {
        _classDescriptor -> release();
        _classDescriptor = NULL;
    }
}

cn::vimfung::luascriptcore::LuaObjectDescriptor* LuaJavaExportClassProxy::getExportClass()
{
    return _classDescriptor;
}

LuaExportNameList LuaJavaExportClassProxy::allExportClassMethods()
{
    LuaExportNameList nameList;

    JNIEnv *env = LuaJavaEnv::getEnv();
    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    if (classImportCls != NULL)
    {
        jmethodID getExportClassMethod = env -> GetStaticMethodID(classImportCls, "_getExportClassMethod", "(Ljava/lang/Class;)[Ljava/lang/String;");
        jobjectArray stringArr = (jobjectArray)env -> CallStaticObjectMethod(classImportCls, getExportClassMethod, (jclass)getExportClass() -> getObject());

        int size = env -> GetArrayLength(stringArr);
        for (int i = 0; i < size; ++i)
        {
            jstring name = (jstring)env -> GetObjectArrayElement(stringArr, i);

            const char* nameCStr = env -> GetStringUTFChars(name, NULL);
            nameList.push_back(nameCStr);
            env -> ReleaseStringUTFChars(name, nameCStr);

            env -> DeleteLocalRef(name);
        }

        env -> DeleteLocalRef(stringArr);
    }

    env -> DeleteLocalRef(classImportCls);
    LuaJavaEnv::resetEnv(env);

    return nameList;
}

LuaExportNameList LuaJavaExportClassProxy::allExportInstanceMethods()
{
    LuaExportNameList nameList;

    JNIEnv *env = LuaJavaEnv::getEnv();
    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    if (classImportCls != NULL)
    {
        jmethodID getExportClassMethod = env -> GetStaticMethodID(classImportCls, "_getExportInstanceMethod", "(Ljava/lang/Class;)[Ljava/lang/String;");
        jobjectArray stringArr = (jobjectArray)env -> CallStaticObjectMethod(classImportCls, getExportClassMethod, (jclass)getExportClass() -> getObject());

        int size = env -> GetArrayLength(stringArr);
        for (int i = 0; i < size; ++i)
        {
            jstring name = (jstring)env -> GetObjectArrayElement(stringArr, i);

            const char* nameCStr = env -> GetStringUTFChars(name, NULL);
            nameList.push_back(nameCStr);
            env -> ReleaseStringUTFChars(name, nameCStr);

            env -> DeleteLocalRef(name);
        }

        env -> DeleteLocalRef(stringArr);
    }

    env -> DeleteLocalRef(classImportCls);
    LuaJavaEnv::resetEnv(env);

    return nameList;
}

LuaExportNameList LuaJavaExportClassProxy::allExportGetterFields()
{
    LuaExportNameList nameList;

    JNIEnv *env = LuaJavaEnv::getEnv();
    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    if (classImportCls != NULL)
    {
        jmethodID getExportClassMethod = env -> GetStaticMethodID(classImportCls, "_getExportFieldMethod", "(Ljava/lang/Class;)[Ljava/lang/String;");
        jobjectArray stringArr = (jobjectArray)env -> CallStaticObjectMethod(classImportCls, getExportClassMethod, (jclass)getExportClass() -> getObject());

        int size = env -> GetArrayLength(stringArr);
        for (int i = 0; i < size; ++i)
        {
            jstring name = (jstring)env -> GetObjectArrayElement(stringArr, i);

            const char* nameCStr = env -> GetStringUTFChars(name, NULL);
            nameList.push_back(nameCStr);
            env -> ReleaseStringUTFChars(name, nameCStr);

            env -> DeleteLocalRef(name);
        }

        env -> DeleteLocalRef(stringArr);
    }

    env -> DeleteLocalRef(classImportCls);
    LuaJavaEnv::resetEnv(env);

    return nameList;
}

LuaExportNameList LuaJavaExportClassProxy::allExportSetterFields()
{
    LuaExportNameList nameList;

    JNIEnv *env = LuaJavaEnv::getEnv();
    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    if (classImportCls != NULL)
    {
        jmethodID getExportClassMethod = env -> GetStaticMethodID(classImportCls, "_getExportFieldMethod", "(Ljava/lang/Class;)[Ljava/lang/String;");
        jobjectArray stringArr = (jobjectArray)env -> CallStaticObjectMethod(classImportCls, getExportClassMethod, (jclass)getExportClass() -> getObject());

        int size = env -> GetArrayLength(stringArr);
        for (int i = 0; i < size; ++i)
        {
            jstring name = (jstring)env -> GetObjectArrayElement(stringArr, i);

            const char* nameCStr = env -> GetStringUTFChars(name, NULL);
            nameList.push_back(nameCStr);
            env -> ReleaseStringUTFChars(name, nameCStr);

            env -> DeleteLocalRef(name);
        }

        env -> DeleteLocalRef(stringArr);
    }

    env -> DeleteLocalRef(classImportCls);
    LuaJavaEnv::resetEnv(env);

    return nameList;
}