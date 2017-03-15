//
// Created by 冯鸿杰 on 16/10/8.
//

#include <stdint.h>
#include <jni.h>
#include <sys/time.h>
#include <stdio.h>
#include "LuaJavaObjectClass.h"
#include "LuaJavaType.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaObjectManager.h"
#include "LuaJavaConverter.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaPointer.h"
#include "LuaJavaObjectInstanceDescriptor.h"
#include "LuaContext.h"
#include "LuaValue.h"

/**
 * 类实例化处理器
 *
 * @param instance 实例对象
 */
static void _luaClassObjectCreated (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass)
{
    using namespace cn::vimfung::luascriptcore;

    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaJavaObjectClass *jobjectClass = (LuaJavaObjectClass *)objectClass;
    jclass cls = jobjectClass -> getModuleClass(env);
    if (cls != NULL)
    {
        //创建实例对象
        jmethodID initMethodId = env->GetMethodID(cls, "<init>", "()V");

        //创建Java层的实例对象
        jobject jcontext = LuaJavaEnv::getJavaLuaContext(env, objectClass->getContext());
        jobject jInstance = env->NewObject(cls, initMethodId);

        LuaJavaObjectInstanceDescriptor *objDesc = new LuaJavaObjectInstanceDescriptor(env, jInstance, jobjectClass);
        //创建Lua中的实例对象
        objectClass -> createLuaInstance(objDesc);

        objDesc -> release();
        env -> DeleteLocalRef(jInstance);
    }

    LuaJavaEnv::resetEnv(env);
}

static void _luaClassObjectDestroy (cn::vimfung::luascriptcore::LuaUserdataRef instance)
{
    using namespace cn::vimfung::luascriptcore;

    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaObjectDescriptor *objDesc = (LuaObjectDescriptor *)instance -> value;
    jobject jInstance = (jobject)objDesc -> getObject();

    LuaJavaEnv::removeAssociateInstance(env, jInstance);

    //移除对象引用
    objDesc -> release();

    LuaJavaEnv::resetEnv(env);
}

static std::string _luaClassObjectDescription (cn::vimfung::luascriptcore::LuaUserdataRef instance)
{
    using namespace cn::vimfung::luascriptcore;

    std::string str;

    JNIEnv *env = LuaJavaEnv::getEnv();

    //表示有实例对象传入
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();

    jclass cls = env -> GetObjectClass(jInstance);
    jmethodID toStringMethodId = env -> GetMethodID(cls, "toString", "()Ljava/lang/String;");

    jstring desc = (jstring)env -> CallObjectMethod(jInstance, toStringMethodId);
    const char *descCStr = env -> GetStringUTFChars(desc, NULL);
    str = descCStr;
    env -> ReleaseStringUTFChars(desc, descCStr);
    env -> DeleteLocalRef(desc);

    env -> DeleteLocalRef(cls);

    LuaJavaEnv::resetEnv(env);

    return str;
}

/**
 * Lua类方法处理器
 *
 * @param module 类模块
 * @param methodName 方法名称
 * @param arguments 参数列表
 *
 * @return 返回值
 */
static LuaValue* _luaClassMethodHandler(LuaModule *module, std::string methodName, LuaArgumentList arguments)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    LuaJavaObjectClass *jmodule = (LuaJavaObjectClass *)module;
    if (jmodule != NULL)
    {
        static jclass luaValueClass = LuaJavaType::luaValueClass(env);

        jclass moduleClass = jmodule -> getModuleClass(env);
        jmethodID invokeMethodID = env -> GetStaticMethodID(LuaJavaType::moduleClass(env), "_methodInvoke", "(Ljava/lang/Class;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
        jstring jMethodName = env -> NewStringUTF(methodName.c_str());

        //参数
        jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
        int index = 0;
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
        {
            LuaValue *argument = *it;
            jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, jmodule -> getContext(), argument);
            env -> SetObjectArrayElement(argumentArr, index, jArgument);
            env -> DeleteLocalRef(jArgument);
            index++;
        }

        jobject result = env -> CallStaticObjectMethod(moduleClass, invokeMethodID, moduleClass, jMethodName, argumentArr);
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, jmodule -> getContext(), result);
        env -> DeleteLocalRef(result);

        env -> DeleteLocalRef(jMethodName);
        env -> DeleteLocalRef(argumentArr);
    }

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}


/**
 * Lua类实例方法处理器
 *
 * @param module 模块对象
 * @param methodName 方法名称
 * @param arguments 方法参数列表
 *
 * @return 方法返回值
 */
static LuaValue* _luaInstanceMethodHandler (cn::vimfung::luascriptcore::LuaUserdataRef instance, cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string methodName, LuaArgumentList arguments)
{
    using namespace cn::vimfung::luascriptcore;
    LuaValue *retValue = NULL;

    JNIEnv *env = LuaJavaEnv::getEnv();

    //转换为Java的实例对象
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();

    jmethodID invokeMethodID = env -> GetMethodID(LuaJavaType::luaObjectClass(env), "_instanceMethodInvoke", "(Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");

    static jclass luaValueClass = LuaJavaType::luaValueClass(env);

    jstring jMethodName = env -> NewStringUTF(methodName.c_str());

    //参数
    jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
    int index = 0;
    for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
    {
        LuaValue *argument = *it;
        jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, objectClass -> getContext(), argument);
        env -> SetObjectArrayElement(argumentArr, index, jArgument);
        env -> DeleteLocalRef(jArgument);
        index++;
    }

    jobject result = env -> CallObjectMethod(jInstance, invokeMethodID, jMethodName, argumentArr);
    retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, objectClass -> getContext(), result);
    env -> DeleteLocalRef(result);

    env -> DeleteLocalRef(jMethodName);
    env -> DeleteLocalRef(argumentArr);

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
 * @param instance 实例对象
 * @param objectClass 实例类型对象
 * @param fieldName 字段名称
 *
 * @return 返回值
 */
static LuaValue* _luaClassGetterHandler (cn::vimfung::luascriptcore::LuaUserdataRef instance, cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string fieldName)
{
    using namespace cn::vimfung::luascriptcore;

    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    //转换为Java的实例对象
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();

    jmethodID getFieldId = env -> GetMethodID(LuaJavaType::luaObjectClass(env), "_getField", "(Ljava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;");

    jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
    jobject retObj = env -> CallObjectMethod(jInstance, getFieldId, fieldNameStr);

    if (retObj != NULL)
    {
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, objectClass -> getContext(), retObj);
        env -> DeleteLocalRef(retObj);
    }

    env -> DeleteLocalRef(fieldNameStr);

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
 * @param instance 实例对象
 * @param objectClass 实例类型对象
 * @param fieldName 字段名称
 * @param value 字段值
 */
static void _luaClassSetterHandler (cn::vimfung::luascriptcore::LuaUserdataRef instance, cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string fieldName, LuaValue *value)
{
    using namespace cn::vimfung::luascriptcore;
    JNIEnv *env = LuaJavaEnv::getEnv();

    //转换为Java的实例对象
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();

    jmethodID setFieldId = env -> GetMethodID(LuaJavaType::luaObjectClass(env), "_setField", "(Ljava/lang/String;Lcn/vimfung/luascriptcore/LuaValue;)V");

    jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());

    jobject jValue = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, objectClass -> getContext(), value);
    env -> CallVoidMethod(jInstance, setFieldId, fieldNameStr, jValue);
    env -> DeleteLocalRef(jValue);

    env -> DeleteLocalRef(fieldNameStr);

    LuaJavaEnv::resetEnv(env);
}

static void _luaSubclassHandler (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string subclassName)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaJavaObjectClass *javaObjectClass = (LuaJavaObjectClass *)objectClass;

    //创建子类描述
    LuaJavaObjectClass *subclass = new LuaJavaObjectClass(
            env,
            javaObjectClass,
            javaObjectClass -> getModuleClass(env),
            NULL,
            NULL,
            NULL);
    objectClass -> getContext() -> registerModule((const std::string)subclassName, subclass);
    subclass -> release();

    LuaJavaEnv::resetEnv(env);
}

LuaJavaObjectClass::LuaJavaObjectClass(JNIEnv *env,
                                       LuaJavaObjectClass *superClass,
                                       jclass moduleClass,
                                       jobjectArray fields,
                                       jobjectArray instanceMethods,
                                       jobjectArray classMethods)
    : LuaObjectClass(superClass)
{
    _moduleClass = (jclass)env -> NewWeakGlobalRef(moduleClass);
    _fields = fields;
    _instanceMethods = instanceMethods;
    _classMethods = classMethods;

    this -> onObjectCreated(_luaClassObjectCreated);
    this -> onObjectDestroy(_luaClassObjectDestroy);
    this -> onObjectGetDescription(_luaClassObjectDescription);
    this -> onSubClass(_luaSubclassHandler);
}

LuaJavaObjectClass::~LuaJavaObjectClass()
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    env -> DeleteWeakGlobalRef(_moduleClass);

    LuaJavaEnv::resetEnv(env);
}

jclass LuaJavaObjectClass::getModuleClass(JNIEnv *env)
{
    if (env -> IsSameObject(_moduleClass, NULL) != JNI_TRUE)
    {
        return _moduleClass;
    }

    return NULL;
}

void LuaJavaObjectClass::onRegister(const std::string &name,
                                    cn::vimfung::luascriptcore::LuaContext *context)
{
    cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onRegister(name, context);

    JNIEnv *env = LuaJavaEnv::getEnv();

    jclass jfieldClass = LuaJavaType::fieldClass(env);
    jmethodID getFieldNameMethodId = env -> GetMethodID(jfieldClass, "getName", "()Ljava/lang/String;");

    jclass jmethodClass = LuaJavaType::methodClass(env);
    jmethodID getMethodNameMethodId = env -> GetMethodID(jmethodClass, "getName", "()Ljava/lang/String;");

    //注册模块字段
    if (_fields != NULL)
    {
        jsize fieldCount = env -> GetArrayLength(_fields);
        for (int i = 0; i < fieldCount; ++i)
        {
            jobject field = env -> GetObjectArrayElement(_fields, i);
            jstring fieldName = (jstring)env -> CallObjectMethod(field, getFieldNameMethodId);

            const char *fieldNameCStr = env -> GetStringUTFChars(fieldName, NULL);
            this -> registerInstanceField(fieldNameCStr, _luaClassGetterHandler, _luaClassSetterHandler);
            env -> ReleaseStringUTFChars(fieldName, fieldNameCStr);

            env -> DeleteLocalRef(fieldName);
            env -> DeleteLocalRef(field);
        }
    }

    //注册实例方法
    if (_instanceMethods != NULL)
    {
        jsize methodCount = env -> GetArrayLength(_instanceMethods);
        for (int i = 0; i < methodCount; ++i)
        {
            jobject method = env -> GetObjectArrayElement(_instanceMethods, i);
            jstring methodName = (jstring)env -> CallObjectMethod(method, getMethodNameMethodId);

            const char *methodNameCStr = env -> GetStringUTFChars(methodName, NULL);
            this -> registerInstanceMethod(methodNameCStr, _luaInstanceMethodHandler);
            env -> ReleaseStringUTFChars(methodName, methodNameCStr);

            env -> DeleteLocalRef(methodName);
            env -> DeleteLocalRef(method);
        }
    }

    //注册类方法
    if (_classMethods != NULL)
    {
        jsize classMethodCount = env -> GetArrayLength(_classMethods);
        for (int i = 0; i < classMethodCount; ++i)
        {
            jobject method = env -> GetObjectArrayElement(_classMethods, i);
            jstring methodName = (jstring)env -> CallObjectMethod(method, getMethodNameMethodId);

            const char *methodNameCStr = env -> GetStringUTFChars(methodName, NULL);
            this -> registerMethod(methodNameCStr, _luaClassMethodHandler);
            env -> ReleaseStringUTFChars(methodName, methodNameCStr);

            env -> DeleteLocalRef(methodName);
            env -> DeleteLocalRef(method);
        }
    }

    LuaJavaEnv::resetEnv(env);

}

bool LuaJavaObjectClass::subclassOf(cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *type)
{
    jboolean isSubclass;

    LuaJavaObjectClass *javaType = dynamic_cast<LuaJavaObjectClass *>(type);
    if (javaType != NULL)
    {
        JNIEnv *env = LuaJavaEnv::getEnv();

        jclass classType = javaType -> getModuleClass(env);
        jclass checkType = this -> getModuleClass(env);

        isSubclass = env -> IsAssignableFrom(checkType, classType);

        LuaJavaEnv::resetEnv(env);
    }

    return isSubclass;
}

void LuaJavaObjectClass::createLuaInstance(cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor *objectDescriptor)
{
    cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::createLuaInstance(objectDescriptor);

    //关联对象
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaJavaEnv::associcateInstance(env, (jobject)objectDescriptor -> getObject(), objectDescriptor);
    LuaJavaEnv::resetEnv(env);
}