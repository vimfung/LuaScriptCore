//
// Created by 冯鸿杰 on 16/10/8.
//

#include <stdint.h>
#include "LuaJavaObjectClass.h"
#include "LuaJavaType.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaObjectManager.h"
#include "LuaJavaConverter.h"

/**
 * 类实例化处理器
 *
 * @param instance 实例对象
 */
static void _luaClassObjectCreated (cn::vimfung::luascriptcore::modules::oo::LuaClassInstance *instance)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    jobject jobjectType = LuaJavaEnv::getJavaLuaModule(env, (LuaModule *)instance -> getObjectClass());
    if (jobjectType != NULL)
    {
        jclass objectClass = env -> GetObjectClass(jobjectType);
        jmethodID initMethodId = env -> GetMethodID(objectClass, "<init>", "()V");

        //创建Java层的实例对象
        jobject jInstance = env -> NewObject(objectClass, initMethodId);
        jobject jGlobalInstance = env -> NewGlobalRef(jInstance);
        //关联到Lua实例对象
        instance -> setField("_nativeObject", LuaValue::PtrValue(jGlobalInstance));

        LuaObjectManager::SharedInstance() -> putObject(instance);

        //创建Java层的类实例对象
        jmethodID instanceInitMethodId = env -> GetMethodID(LuaJavaType::objectClassInstanceClass(env), "<init>", "(I)V");
        jobject classInstance = env -> NewObject(LuaJavaType::objectClassInstanceClass(env), instanceInitMethodId, instance -> objectId());

        //回调Java层中的实例方法
        jmethodID instanceInitializeMethodId = env -> GetMethodID(objectClass, "instanceInitialize", "(Lcn/vimfung/luascriptcore/modules/oo/LuaClassInstance;)V");
        env -> CallVoidMethod(jGlobalInstance, instanceInitializeMethodId, classInstance);

        //主动取消关联
        LuaObjectManager::SharedInstance() -> removeObject(instance -> objectId());
    }

    LuaJavaEnv::resetEnv(env);
}

static void _luaClassObjectDestroy (cn::vimfung::luascriptcore::modules::oo::LuaClassInstance *instance)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaValue *nativeObjectValue = instance -> getField("_nativeObject");
    jobject jInstance = (jobject)nativeObjectValue -> toPtr();

    LuaObjectManager::SharedInstance() -> putObject(instance);

    //创建Java层的类实例对象
    jmethodID instanceInitMethodId = env -> GetMethodID(LuaJavaType::objectClassInstanceClass(env), "<init>", "(I)V");
    jobject classInstance = env -> NewObject(LuaJavaType::objectClassInstanceClass(env), instanceInitMethodId, instance -> objectId());

    //回调Java层中的实例方法
    jmethodID instanceUninitializeMethodId = env -> GetMethodID(env -> GetObjectClass(jInstance), "instanceUninitialize", "(Lcn/vimfung/luascriptcore/modules/oo/LuaClassInstance;)V");
    env -> CallVoidMethod(jInstance, instanceUninitializeMethodId, classInstance);

    //释放Java层对象
    env -> DeleteGlobalRef(jInstance);

    //主动取消关联
    LuaObjectManager::SharedInstance() -> removeObject(instance -> objectId());

    LuaJavaEnv::resetEnv(env);
}

static std::string _luaClassObjectDescription (cn::vimfung::luascriptcore::modules::oo::LuaClassInstance *instance)
{
    std::string str;

    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaValue *nativeObjectValue = instance -> getField("_nativeObject");
    LOGI("object value type = %d", nativeObjectValue -> getType());
    jobject jInstance = (jobject)nativeObjectValue -> toPtr();

    LuaObjectManager::SharedInstance() -> putObject(instance);

    //创建Java层的类实例对象
    jmethodID instanceInitMethodId = env -> GetMethodID(LuaJavaType::objectClassInstanceClass(env), "<init>", "(I)V");
    jobject classInstance = env -> NewObject(LuaJavaType::objectClassInstanceClass(env), instanceInitMethodId, instance -> objectId());

    //回调Java层中的实例方法
    jmethodID descMethodId = env -> GetMethodID(env -> GetObjectClass(jInstance), "instanceDescription", "(Lcn/vimfung/luascriptcore/modules/oo/LuaClassInstance;)Ljava/lang/String;");
    jstring objDesc = (jstring)env -> CallObjectMethod(jInstance, descMethodId, classInstance);
    if (objDesc != NULL)
    {
        const char *objDescCStr = env -> GetStringUTFChars(objDesc, NULL);
        str = objDescCStr;
        env -> ReleaseStringUTFChars(objDesc, objDescCStr);
    }

    //主动取消关联
    LuaObjectManager::SharedInstance() -> removeObject(instance -> objectId());

    LuaJavaEnv::resetEnv(env);

    return str;
}

/**
 * Lua类方法处理器
 *
 * @param module 模块对象
 * @param methodName 方法名称
 * @param arguments 方法参数列表
 *
 * @return 方法返回值
 */
static LuaValue* _luaClassMethodHandler (cn::vimfung::luascriptcore::modules::oo::LuaClassInstance *instance, std::string methodName, LuaArgumentList arguments)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    LuaValue *nativeObjectValue = instance -> getField("_nativeObject");
    jobject jInstance = (jobject)nativeObjectValue -> toPtr();

    if (jInstance != NULL)
    {
        jclass objectClass = env -> GetObjectClass(jInstance);
        jmethodID invokeMethodID = env -> GetMethodID(objectClass, "_methodInvoke", "(Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
        static jclass luaValueClass = LuaJavaType::luaValueClass(env);

        jstring jMethodName = env -> NewStringUTF(methodName.c_str());

        //参数
        jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
        int index = 0;
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
        {
            LuaValue *argument = *it;
            jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, argument);
            env -> SetObjectArrayElement(argumentArr, index, jArgument);
            index++;
        }

        jobject result = env -> CallObjectMethod(jInstance, invokeMethodID, jMethodName, argumentArr);
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, result);
    }

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

/**
 * 类获取器处理器
 *
 * @param module 模块对象
 * @param fieldName 字段名称
 *
 * @return 返回值
 */
static LuaValue* _luaClassGetterHandler (cn::vimfung::luascriptcore::modules::oo::LuaClassInstance *instance, std::string fieldName)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    LuaValue *nativeObjectValue = instance -> getField("_nativeObject");
    jobject jInstance = (jobject)nativeObjectValue -> toPtr();

    if (jInstance != NULL)
    {
        jclass objectClass = env -> GetObjectClass(jInstance);
        jmethodID getFieldId = env -> GetMethodID(objectClass, "_getField", "(Ljava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;");

        jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
        jobject retObj = env -> CallObjectMethod(jInstance, getFieldId, fieldNameStr);

        if (retObj != NULL)
        {
            retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, retObj);
        }
    }

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

/**
 * 类设置器处理器
 *
 * @param module 模块对象
 * @param fieldName 字段名称
 * @param value 字段值
 */
static void _luaClassSetterHandler (cn::vimfung::luascriptcore::modules::oo::LuaClassInstance *instance, std::string fieldName, LuaValue *value)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaValue *nativeObjectValue = instance -> getField("_nativeObject");
    jobject jInstance = (jobject)nativeObjectValue -> toPtr();
    if (jInstance != NULL)
    {
        jclass objectClass = env -> GetObjectClass(jInstance);
        jmethodID setFieldId = env -> GetMethodID(objectClass, "_setField", "(Ljava/lang/String;Lcn/vimfung/luascriptcore/LuaValue;)V");

        jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
        env -> CallVoidMethod(jInstance, setFieldId, fieldNameStr, LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, value));
    }

    LuaJavaEnv::resetEnv(env);
}

LuaJavaObjectClass::LuaJavaObjectClass(JNIEnv *env,
                                       const std::string &superClassName,
                                       jclass moduleClass,
                                       jobjectArray fields,
                                       jobjectArray methods)
    : LuaObjectClass(superClassName)
{
    _env = env;
    _moduleClass = moduleClass;
    _fields = fields;
    _methods = methods;

    this -> onObjectCreated(_luaClassObjectCreated);
    this -> onObjectDestroy(_luaClassObjectDestroy);
    this -> onObjectGetDescription(_luaClassObjectDescription);
}

void LuaJavaObjectClass::onRegister(const std::string &name,
                                    cn::vimfung::luascriptcore::LuaContext *context)
{
    cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onRegister(name, context);

    LOGI("start on java class register...");

    //注册模块字段
    jsize fieldCount = _env -> GetArrayLength(_fields);
    jclass jfieldClass = LuaJavaType::fieldClass(_env);
    jmethodID getFieldNameMethodId = _env -> GetMethodID(jfieldClass, "getName", "()Ljava/lang/String;");
    for (int i = 0; i < fieldCount; ++i)
    {
        jobject field = _env -> GetObjectArrayElement(_fields, i);
        jstring fieldName = (jstring)_env -> CallObjectMethod(field, getFieldNameMethodId);

        const char *fieldNameCStr = _env -> GetStringUTFChars(fieldName, NULL);
        this -> registerInstanceField(fieldNameCStr, _luaClassGetterHandler, _luaClassSetterHandler);
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
        this -> registerInstanceMethod(methodNameCStr, _luaClassMethodHandler);
        _env -> ReleaseStringUTFChars(methodName, methodNameCStr);
    }
}