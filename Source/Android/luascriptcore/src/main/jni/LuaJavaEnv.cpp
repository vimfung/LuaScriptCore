//
// Created by 冯鸿杰 on 16/9/29.
//

#include <stdint.h>
#include <map>
#include "LuaJavaEnv.h"
#include "LuaObjectManager.h"
#include "LuaJavaType.h"
#include "LuaJavaConverter.h"
#include "LuaJavaModule.h"
#include "LuaDefine.h"

/**
 * Java虚拟机对象
 */
static JavaVM *_javaVM;

/**
 * 附加线程标识
 */
static bool _attatedThread;

/**
 * Java中上下文对象集合
 */
static std::map<jint, jobject> _javaObjectMap;

/**
 * Lua方法处理器
 *
 * @param context 上下文对象
 * @param methodName 方法名称
 * @param arguments 方法参数
 *
 * @returns 返回值
 */
static LuaValue* _luaMethodHandler (LuaContext *context, std::string methodName, LuaArgumentList arguments)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    jobject jcontext = LuaJavaEnv::getJavaLuaContext(env, context);
    if (jcontext != NULL)
    {
        if (env -> IsSameObject(jcontext, NULL) != JNI_TRUE)
        {
            static jclass contenxtClass = LuaJavaType::contextClass(env);
            static jmethodID invokeMethodID = env -> GetMethodID(contenxtClass, "methodInvoke", "(Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
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

            jobject result = env -> CallObjectMethod(jcontext, invokeMethodID, jMethodName, argumentArr);
            retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, result);
        }
    }

    LuaJavaEnv::resetEnv(env);

    return retValue;
}

/**
 * Lua模块方法处理器
 *
 * @param module 模块对象
 * @param methodName 方法名称
 * @param arguments 方法参数列表
 *
 * @return 方法返回值
 */
static LuaValue* _luaModuleMethodHandler (LuaModule *module, std::string methodName, LuaArgumentList arguments)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    jobject jmodule = LuaJavaEnv::getJavaLuaModule(env, (LuaJavaModule *)module);
    if (jmodule != NULL)
    {
        static jclass moduleClass = LuaJavaType::moduleClass(env);
        static jmethodID invokeMethodID = env -> GetMethodID(moduleClass, "methodInvoke", "(Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
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

        jobject result = env -> CallObjectMethod(jmodule, invokeMethodID, jMethodName, argumentArr);
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, result);
    }

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

static LuaValue* _luaModuleGetterHandler (LuaModule *module, std::string fieldName)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    jobject jmodule = LuaJavaEnv::getJavaLuaModule(env, (LuaJavaModule *)module);
    if (jmodule != NULL)
    {
        static jclass moduleClass = LuaJavaType::moduleClass(env);
        static jmethodID getFieldId = env -> GetMethodID(moduleClass, "getField", "(Ljava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;");

        jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
        jobject retObj = env -> CallObjectMethod(jmodule, getFieldId, fieldNameStr);

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

static void _luaModuleSetterHandler (LuaModule *module, std::string fieldName, LuaValue *value)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    jobject jmodule = LuaJavaEnv::getJavaLuaModule(env, (LuaJavaModule *)module);
    if (jmodule != NULL)
    {
        static jclass moduleClass = LuaJavaType::moduleClass(env);
        static jmethodID setFieldId = env -> GetMethodID(moduleClass, "setField", "(Ljava/lang/String;Lcn/vimfung/luascriptcore/LuaValue;)V");

        jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
        env -> CallVoidMethod(jmodule, setFieldId, fieldNameStr, LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, value));
    }

    LuaJavaEnv::resetEnv(env);
}

void LuaJavaEnv::init(JavaVM *javaVM)
{
    _javaVM = javaVM;
    _attatedThread = false;
}

JNIEnv* LuaJavaEnv::getEnv()
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
        _attatedThread = true;
    }

    return envnow;
}

void LuaJavaEnv::resetEnv(JNIEnv *env)
{
    if(_attatedThread)
    {
        _javaVM -> DetachCurrentThread();
        _attatedThread = false;
    }
}

jobject LuaJavaEnv::createJavaLuaContext(JNIEnv *env, LuaContext *context)
{
    static jclass contextClass = LuaJavaType::contextClass(env);
    static jmethodID initMethodId = env -> GetMethodID(contextClass, "<init>", "(I)V");

    int nativeId = LuaObjectManager::SharedInstance() -> putObject(context);
    jobject jcontext = env -> NewObject(contextClass, initMethodId, nativeId);

    _javaObjectMap[context -> objectId()] = env -> NewWeakGlobalRef(jcontext);

    return jcontext;
}

jobject LuaJavaEnv::getJavaLuaContext(JNIEnv *env, LuaContext *context)
{
    std::map<jint, jobject>::iterator it =  _javaObjectMap.find(context->objectId());
    if (it != _javaObjectMap.end())
    {
        jobject jcontext = it -> second;
        if (env -> IsSameObject(jcontext, NULL) != JNI_TRUE)
        {
            return jcontext;
        }
        else
        {
            //如果Java中已释放对象则在集合中进行移除
            env -> DeleteWeakGlobalRef(jcontext);
            _javaObjectMap.erase(it);
        }
    }

    return NULL;
}

jobject LuaJavaEnv::createJavaLuaModule(JNIEnv *env, jclass moduleClass, LuaModule *module)
{
    int nativeId = LuaObjectManager::SharedInstance() -> putObject(module);

    //创建原生层Module对象
    jmethodID initMethodId = env -> GetMethodID(moduleClass, "<init>", "()V");
    jobject jmodule = env -> NewObject(moduleClass, initMethodId);

    //关联本地对象
    jfieldID nativeIdFieldId = env -> GetFieldID(moduleClass, "_nativeId", "I");
    env -> SetIntField(jmodule, nativeIdFieldId, nativeId);

    //放入关联表
    _javaObjectMap[nativeId] = env -> NewWeakGlobalRef(jmodule);

    return jmodule;
}

jobject LuaJavaEnv::getJavaLuaModule(JNIEnv *env, LuaModule *module)
{
    std::map<jint, jobject>::iterator it =  _javaObjectMap.find(module->objectId());
    if (it != _javaObjectMap.end())
    {
        jobject jmodule = it -> second;
        if (env -> IsSameObject(jmodule, NULL) != JNI_TRUE)
        {
            return jmodule;
        }
        else
        {
            //如果Java中已释放对象则在集合中进行移除
            env -> DeleteWeakGlobalRef(jmodule);
            _javaObjectMap.erase(it);
        }
    }

    return NULL;
}

jobject LuaJavaEnv::releaseObject(JNIEnv *env, jint objectId)
{
    std::map<jint, jobject>::iterator it =  _javaObjectMap.find(objectId);
    if (it != _javaObjectMap.end())
    {
        //为LuaContext对象,解除对象引用
        env -> DeleteWeakGlobalRef(it -> second);
        _javaObjectMap.erase(it);
    }

    LuaObjectManager::SharedInstance() -> removeObject(objectId);
}

LuaMethodHandler LuaJavaEnv::luaMethodHandler()
{
    return (LuaMethodHandler) _luaMethodHandler;
}

LuaModuleMethodHandler LuaJavaEnv::luaModuleMethodHandler()
{
    return (LuaModuleMethodHandler) _luaModuleMethodHandler;
}

LuaModuleSetterHandler LuaJavaEnv::luaModuleSetterHandler()
{
    return (LuaModuleSetterHandler) _luaModuleSetterHandler;
}

LuaModuleGetterHandler LuaJavaEnv::luaModuleGetterHandler()
{
    return (LuaModuleGetterHandler) _luaModuleGetterHandler;
}
