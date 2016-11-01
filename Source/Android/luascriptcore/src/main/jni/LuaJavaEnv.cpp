//
// Created by 冯鸿杰 on 16/9/29.
//

#include <stdint.h>
#include <map>
#include <set>
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
 * 对象实例的lua引用对照表
 */
static std::map<long, void**> _instanceMap;

/**
 * 对象实例的缓存集合
 */
static std::set<jobject> _instanceSet;

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
                jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, argument);
                env -> SetObjectArrayElement(argumentArr, index, jArgument);
                index++;
            }

            jobject result = env -> CallObjectMethod(jcontext, invokeMethodID, jMethodName, argumentArr);
            if (result != NULL)
            {
                retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, result);
            }
            else
            {
                retValue = new LuaValue();
            }
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

    LuaJavaModule *jmodule = (LuaJavaModule *)module;
    if (jmodule != NULL)
    {
        static jclass moduleClass = jmodule -> getModuleClass();
        static jmethodID invokeMethodID = env -> GetStaticMethodID(moduleClass, "_methodInvoke", "(Ljava/lang/Class;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
        static jclass luaValueClass = LuaJavaType::luaValueClass(env);

        jstring jMethodName = env -> NewStringUTF(methodName.c_str());

        //参数
        jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
        int index = 0;
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
        {
            LuaValue *argument = *it;
            jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, jmodule -> getContext(), argument);
            env -> SetObjectArrayElement(argumentArr, index, jArgument);
            index++;
        }

        jobject result = env -> CallStaticObjectMethod(moduleClass, invokeMethodID, moduleClass, jMethodName, argumentArr);
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, result);
    }

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
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

void LuaJavaEnv::associcateInstance(jobject instance, void **ref)
{
    long key = (long)instance;
    std::map<long, void**>::iterator it =  _instanceMap.find(key);
    if (it == _instanceMap.end())
    {
        _instanceMap[key] = ref;
    }

    //保存实例到集合中
    _instanceSet.insert(instance);
}

void LuaJavaEnv::removeAssociateInstance(jobject instance, void **ref)
{
    long key = (long)instance;
    std::map<long, void**>::iterator it =  _instanceMap.find(key);
    if (it != _instanceMap.end())
    {
        _instanceMap.erase(it);
    }

    std::set<jobject>::iterator setIt = _instanceSet.find(instance);
    if (setIt != _instanceSet.end())
    {
        _instanceSet.erase(setIt);
    }

}

void** LuaJavaEnv::getAssociateInstanceRef(jobject instance)
{
    long key = 0;
    std::set<jobject>::iterator setIt = _instanceSet.find(instance);
    if (setIt == _instanceSet.end())
    {
        JNIEnv *env = LuaJavaEnv::getEnv();

        //没找到对应的实例，则使用遍历方式对比对象
        for (std::set<jobject>::iterator it = _instanceSet.begin(); it != _instanceSet.end() ; ++it)
        {
            if (env -> IsSameObject(instance, (jobject)*it) == JNI_TRUE)
            {
                key = (long)*it;
                break;
            }
        }

        LuaJavaEnv::resetEnv(env);
    }
    else
    {
        key = (long)instance;
    }

    std::map<long, void**>::iterator it =  _instanceMap.find(key);
    if (it != _instanceMap.end())
    {
        return it -> second;
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
