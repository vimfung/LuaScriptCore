//
// Created by 冯鸿杰 on 16/10/8.
//

#include <stdint.h>
#include <jni.h>
#include "LuaJavaObjectClass.h"
#include "LuaJavaType.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaObjectManager.h"
#include "LuaJavaConverter.h"
#include "LuaJavaObjectDescriptor.h"

/**
 * 类实例化处理器
 *
 * @param instance 实例对象
 */
static void _luaClassObjectCreated (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaJavaObjectClass *jobjectClass = (LuaJavaObjectClass *)objectClass;
    jclass cls = jobjectClass -> getModuleClass();
    if (cls != NULL)
    {
        lua_State *state = jobjectClass->getContext()->getLuaState();

        //创建实例对象
        jmethodID initMethodId = env->GetMethodID(cls, "<init>",
                                                  "(Lcn/vimfung/luascriptcore/LuaContext;)V");

        //创建Java层的实例对象
        jobject jcontext = LuaJavaEnv::getJavaLuaContext(env, objectClass->getContext());
        jobject jInstance = env->NewObject(cls, initMethodId, jcontext);

        LuaJavaObjectDescriptor *objDesc = new LuaJavaObjectDescriptor(jInstance);
        objDesc -> setUserdata(objectClass);

        //先为实例对象在lua中创建内存
        LuaJavaObjectDescriptor **ref = (LuaJavaObjectDescriptor **) lua_newuserdata(state, sizeof(LuaJavaObjectDescriptor *));
        *ref = objDesc;

        luaL_getmetatable(state, jobjectClass->getName().c_str());
        if (lua_istable(state, -1))
        {
            lua_setmetatable(state, -2);
        }
        else
        {
            lua_pop(state, 1);
        }

        //关联对象
        LuaJavaEnv::associcateInstance((jobject)objDesc -> getObject(), (void **)ref);

        //调用实例对象的init方法
        lua_pushvalue(state, 1);
        lua_getfield(state, -1, "init");
        if (lua_isfunction(state, -1))
        {
            lua_pushvalue(state, 1);
            lua_pcall(state, 1, 0, 0);
            lua_pop(state, 1);
        }
        else
        {
            lua_pop(state, 2);
        }
    }

    LuaJavaEnv::resetEnv(env);
}

static void _luaClassObjectDestroy (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    lua_State *state = objectClass -> getContext() -> getLuaState();

    if (lua_gettop(state) > 0 && lua_isuserdata(state, 1))
    {
        //表示有实例对象传入
        LuaJavaObjectDescriptor **ref = (LuaJavaObjectDescriptor **)lua_touserdata(state, 1);
        jobject instance = (jobject)(*ref) -> getObject();

        LuaJavaEnv::removeAssociateInstance(instance, (void **)ref);

        //调用实例对象的destroy方法
        lua_pushvalue(state, 1);
        lua_getfield(state, -1, "destroy");
        if (lua_isfunction(state, -1))
        {
            lua_pushvalue(state, 1);
            lua_pcall(state, 1, 0, 0);
        }
        lua_pop(state, 2);

        //移除Java实例对象引用
        (*ref) -> release();
    }

    LuaJavaEnv::resetEnv(env);
}

static std::string _luaClassObjectDescription (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass)
{
    std::string str;

    JNIEnv *env = LuaJavaEnv::getEnv();

    lua_State *state = objectClass -> getContext() -> getLuaState();

    if (lua_gettop(state) > 0 && lua_isuserdata(state, 1))
    {
        //表示有实例对象传入
        LuaJavaObjectDescriptor **ref = (LuaJavaObjectDescriptor **)lua_touserdata(state, 1);
        jobject instance = (jobject)(*ref) -> getObject();

        jclass cls = env -> GetObjectClass(instance);
        jmethodID toStringMethodId = env -> GetMethodID(cls, "toString", "()Ljava/lang/String;");

        jstring desc = (jstring)env -> CallObjectMethod(instance, toStringMethodId);
        const char *descCStr = env -> GetStringUTFChars(desc, NULL);
        str = descCStr;
        env -> ReleaseStringUTFChars(desc, descCStr);
    }

    LuaJavaEnv::resetEnv(env);

    return str;
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
static LuaValue* _luaInstanceMethodHandler (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string methodName, LuaArgumentList arguments)
{
    LuaValue *retValue = NULL;

    JNIEnv *env = LuaJavaEnv::getEnv();

    lua_State *state = objectClass -> getContext() -> getLuaState();

    if (lua_gettop(state) > 0 && lua_isuserdata(state, 1))
    {
        //表示有实例对象传入
        LuaJavaObjectDescriptor **ref = (LuaJavaObjectDescriptor **)lua_touserdata(state, 1);
        jobject instance = (jobject)(*ref) -> getObject();

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
            index++;
        }

        jobject result = env -> CallObjectMethod(instance, invokeMethodID, jMethodName, argumentArr);
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
static LuaValue* _luaClassGetterHandler (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string fieldName)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    lua_State *state = objectClass -> getContext() -> getLuaState();
    if (lua_gettop(state) > 0 && lua_isuserdata(state, 1))
    {
        //表示有实例对象传入
        LuaJavaObjectDescriptor **ref = (LuaJavaObjectDescriptor **)lua_touserdata(state, 1);
        jobject instance = (jobject)(*ref) -> getObject();

        jmethodID getFieldId = env -> GetMethodID(LuaJavaType::luaObjectClass(env), "_getField", "(Ljava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;");

        jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
        jobject retObj = env -> CallObjectMethod(instance, getFieldId, fieldNameStr);

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
static void _luaClassSetterHandler (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string fieldName, LuaValue *value)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    lua_State *state = objectClass -> getContext() -> getLuaState();

    if (lua_gettop(state) > 0 && lua_isuserdata(state, 1))
    {
        //表示有实例对象传入
        LuaJavaObjectDescriptor **ref = (LuaJavaObjectDescriptor **)lua_touserdata(state, 1);
        jobject instance = (jobject)(*ref) -> getObject();

        jmethodID setFieldId = env -> GetMethodID(LuaJavaType::luaObjectClass(env), "_setField", "(Ljava/lang/String;Lcn/vimfung/luascriptcore/LuaValue;)V");

        jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
        env -> CallVoidMethod(instance, setFieldId, fieldNameStr, LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, objectClass -> getContext(), value));
    }

    LuaJavaEnv::resetEnv(env);
}

static void _luaSubclassHandler (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass, std::string subclassName)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    LuaJavaObjectClass *javaObjectClass = (LuaJavaObjectClass *)objectClass;

    //创建子类描述
    LuaJavaObjectClass *subclass = new LuaJavaObjectClass(
            env,
            (const std::string)javaObjectClass -> getName(),
            javaObjectClass -> getModuleClass(),
            NULL,
            NULL,
            NULL);
    objectClass -> getContext() -> registerModule((const std::string)subclassName, subclass);
    subclass -> release();

    LuaJavaEnv::resetEnv(env);
}

LuaJavaObjectClass::LuaJavaObjectClass(JNIEnv *env,
                                       const std::string &superClassName,
                                       jclass moduleClass,
                                       jobjectArray fields,
                                       jobjectArray instanceMethods,
                                       jobjectArray classMethods)
    : LuaObjectClass(superClassName)
{
    _env = env;
    _moduleClass = (jclass)env -> NewWeakGlobalRef(moduleClass);
    _fields = fields;
    _instanceMethods = instanceMethods;
    _classMethods = classMethods;

    this -> onObjectCreated(_luaClassObjectCreated);
    this -> onObjectDestroy(_luaClassObjectDestroy);
    this -> onObjectGetDescription(_luaClassObjectDescription);
    this -> onSubClass(_luaSubclassHandler);
}

jclass LuaJavaObjectClass::getModuleClass()
{
    if (_env -> IsSameObject(_moduleClass, NULL) != JNI_TRUE)
    {
        return _moduleClass;
    }

    return NULL;
}

void LuaJavaObjectClass::onRegister(const std::string &name,
                                    cn::vimfung::luascriptcore::LuaContext *context)
{
    cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onRegister(name, context);

    jclass jfieldClass = LuaJavaType::fieldClass(_env);
    jmethodID getFieldNameMethodId = _env -> GetMethodID(jfieldClass, "getName", "()Ljava/lang/String;");

    jclass jmethodClass = LuaJavaType::methodClass(_env);
    jmethodID getMethodNameMethodId = _env -> GetMethodID(jmethodClass, "getName", "()Ljava/lang/String;");

    //注册模块字段
    if (_fields != NULL)
    {
        jsize fieldCount = _env -> GetArrayLength(_fields);
        for (int i = 0; i < fieldCount; ++i)
        {
            jobject field = _env -> GetObjectArrayElement(_fields, i);
            jstring fieldName = (jstring)_env -> CallObjectMethod(field, getFieldNameMethodId);

            const char *fieldNameCStr = _env -> GetStringUTFChars(fieldName, NULL);
            this -> registerInstanceField(fieldNameCStr, _luaClassGetterHandler, _luaClassSetterHandler);
            _env -> ReleaseStringUTFChars(fieldName, fieldNameCStr);
        }
    }

    //注册实例方法
    if (_instanceMethods != NULL)
    {
        jsize methodCount = _env -> GetArrayLength(_instanceMethods);
        for (int i = 0; i < methodCount; ++i)
        {
            jobject method = _env -> GetObjectArrayElement(_instanceMethods, i);
            jstring methodName = (jstring)_env -> CallObjectMethod(method, getMethodNameMethodId);

            const char *methodNameCStr = _env -> GetStringUTFChars(methodName, NULL);
            this -> registerInstanceMethod(methodNameCStr, _luaInstanceMethodHandler);
            _env -> ReleaseStringUTFChars(methodName, methodNameCStr);
        }
    }

    //注册类方法
    if (_classMethods != NULL)
    {
        jsize classMethodCount = _env -> GetArrayLength(_classMethods);
        for (int i = 0; i < classMethodCount; ++i)
        {
            jobject method = _env -> GetObjectArrayElement(_classMethods, i);
            jstring methodName = (jstring)_env -> CallObjectMethod(method, getMethodNameMethodId);

            const char *methodNameCStr = _env -> GetStringUTFChars(methodName, NULL);
            this -> registerMethod(methodNameCStr, LuaJavaEnv::luaModuleMethodHandler());
            _env -> ReleaseStringUTFChars(methodName, methodNameCStr);
        }
    }

}