//
// Created by 冯鸿杰 on 16/9/28.
//

#include <stdint.h>
#include "LuaJavaModule.h"
#include "LuaJavaType.h"
#include "LuaJavaEnv.h"

LuaJavaModule::LuaJavaModule(
        JNIEnv *env,
        jclass moduleClass,
        jobjectArray fields,
        jobjectArray methods)
{
    _env = env;
    _moduleClass = moduleClass;
    _fields = fields;
    _methods = methods;
}

void LuaJavaModule::onRegister(const std::string &name, cn::vimfung::luascriptcore::LuaContext *context)
{
    cn::vimfung::luascriptcore::LuaModule::onRegister(name, context);

    //注册模块字段
    jsize fieldCount = _env -> GetArrayLength(_fields);
    jclass jfieldClass = LuaJavaType::fieldClass(_env);
    jmethodID getFieldNameMethodId = _env -> GetMethodID(jfieldClass, "getName", "()Ljava/lang/String;");
    for (int i = 0; i < fieldCount; ++i)
    {
        jobject field = _env -> GetObjectArrayElement(_fields, i);
        jstring fieldName = (jstring)_env -> CallObjectMethod(field, getFieldNameMethodId);

        const char *fieldNameCStr = _env -> GetStringUTFChars(fieldName, NULL);
        this -> registerField(fieldNameCStr, LuaJavaEnv::luaModuleGetterHandler(), LuaJavaEnv::luaModuleSetterHandler());
        _env -> ReleaseStringUTFChars(fieldName, fieldNameCStr);
    }

    //注册模块方法
    jsize methodCount = _env -> GetArrayLength(_methods);
    jclass jmethodClass = LuaJavaType::methodClass(_env);
    jmethodID getMethodNameMethodId = _env -> GetMethodID(jmethodClass, "getName", "()Ljava/lang/String;");
    for (int i = 0; i < methodCount; ++i)
    {
        jobject method = _env -> GetObjectArrayElement(_methods, i);
        jstring methodName = (jstring)_env -> CallObjectMethod(method, getMethodNameMethodId);

        const char *methodNameCStr = _env -> GetStringUTFChars(methodName, NULL);
        this -> registerMethod(methodNameCStr, LuaJavaEnv::luaModuleMethodHandler());
        _env -> ReleaseStringUTFChars(methodName, methodNameCStr);
    }
}
