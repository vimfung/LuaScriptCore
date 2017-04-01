//
// Created by 冯鸿杰 on 17/3/27.
//

#include <StringUtils.h>
#include "LuaJavaClassImport.h"
#include "LuaJavaEnv.h"
#include "LuaJavaExportClassProxy.h"
#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaType.h"
#include "LuaJavaConverter.h"
#include "LuaDefine.h"

using namespace cn::vimfung::luascriptcore::modules::oo;
using namespace cn::vimfung::luascriptcore;

//导出类型列表
static std::list<jclass> _exportClassList;

/**
 * Java对象入栈过滤
 *
 * @param context 上下文对象
 * @param objectDescriptor 需要入栈的对象描述
 */
static bool _javaObjectPushFilter(LuaContext *context, LuaObjectDescriptor *objectDescriptor)
{
    bool filted = false;
    lua_State *state = context -> getLuaState();

    LuaJavaObjectDescriptor *javaObjectDescriptor = dynamic_cast<LuaJavaObjectDescriptor *>(objectDescriptor);
    if (javaObjectDescriptor != NULL)
    {
        JNIEnv *env = LuaJavaEnv::getEnv();

        jobject jObj = (jobject)objectDescriptor -> getObject();
        jclass jCls = env -> GetObjectClass(jObj);

        //获取类名
        std::string className = LuaJavaEnv::getJavaClassName(env, jCls, false);
        filted = LuaClassImport::setLuaMetatable(context, className, objectDescriptor);

        env -> DeleteLocalRef(jCls);

        LuaJavaEnv::resetEnv(env);
    }

    return filted;
}

/**
 * 是否允许导出该类型
 *
 * @param className 类型名称
 *
 * @return true 允许导出，false 不允许导出
 */
static bool _allowExportsClassHandler (const std::string &className)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    //将className由.描述缓存/描述
    std::string clsName = StringUtils::replace(className, ".", "/");
    jclass cls = env -> FindClass(clsName.c_str());

    bool allow = false;
    for (std::list<jclass>::iterator it = _exportClassList.begin(); it != _exportClassList.end() ; ++it)
    {
        jclass targetCls = *it;
        if (env -> IsSameObject(cls, targetCls))
        {
            allow = true;
            break;
        }
    }

    env -> DeleteLocalRef(cls);

    LuaJavaEnv::resetEnv(env);

    return allow;
}

/**
 * 导出类型
 *
 * @param className 类型名称
 *
 * @return 导出类型代理
 */
static LuaExportClassProxy* _exportClassHandler (const std::string &className)
{
    return new LuaJavaExportClassProxy(className);
}

/**
 * 创建实例类型
 *
 * @param classDescriptor 类描述
 *
 * @return 实例对象描述
 */
static LuaObjectDescriptor* _createInstanceHandler (LuaObjectDescriptor *classDescriptor)
{
    LuaJavaObjectDescriptor *objectDescriptor = NULL;

    JNIEnv *env = LuaJavaEnv::getEnv();

    jclass cls = (jclass)classDescriptor -> getObject();
    jmethodID initMethodId = env -> GetMethodID(cls, "<init>", "()V");
    if (initMethodId != NULL)
    {
        jobject instance = env -> NewObject(cls, initMethodId);
        objectDescriptor = new LuaJavaObjectDescriptor(env, instance);
        env -> DeleteLocalRef(instance);
    }

    LuaJavaEnv::resetEnv(env);

    return objectDescriptor;
}

/**
 * 类方法调用处理器
 *
 * @param classDescriptor 类描述
 * @param methodName 方法名
 * @param args 参数列表
 *
 * @return 返回值
 */
static LuaValue* _classMethodInvokeHandler (LuaContext *context, LuaObjectDescriptor *classDescriptor, std::string methodName, LuaArgumentList arguments)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    jclass moduleClass = (jclass)classDescriptor -> getObject();


    static jclass luaValueClass = LuaJavaType::luaValueClass(env);

    jmethodID invokeMethodID = env -> GetStaticMethodID(classImportCls, "_classMethodInvoke", "(Ljava/lang/Class;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");
    jstring jMethodName = env -> NewStringUTF(methodName.c_str());

    //参数
    jobjectArray argumentArr = env -> NewObjectArray(arguments.size(), luaValueClass, NULL);
    int index = 0;
    for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); it ++)
    {
        LuaValue *argument = *it;
        jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, argument);
        env -> SetObjectArrayElement(argumentArr, index, jArgument);
        env -> DeleteLocalRef(jArgument);
        index++;
    }

    jobject result = env -> CallStaticObjectMethod(classImportCls, invokeMethodID, moduleClass, jMethodName, argumentArr);
    retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, result);
    env -> DeleteLocalRef(result);

    env -> DeleteLocalRef(jMethodName);
    env -> DeleteLocalRef(argumentArr);
    env -> DeleteLocalRef(classImportCls);

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

/**
 * 实例方法调用处理器
 *
 * @param classDescriptor 类描述
 * @param instance 实例对象
 * @param methodName 方法名
 * @param args 参数列表
 *
 * @return 返回值
 */
static LuaValue* _instanceMethodInvokeHandler (LuaContext *context, LuaObjectDescriptor *classDescriptor, LuaUserdataRef instance, std::string methodName, LuaArgumentList args)
{
    LuaValue *retValue = NULL;

    JNIEnv *env = LuaJavaEnv::getEnv();

    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    jclass moduleClass = (jclass)classDescriptor -> getObject();

    //转换为Java的实例对象
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();
    jmethodID invokeMethodID = env -> GetStaticMethodID(classImportCls, "_instanceMethodInvoke", "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/String;[Lcn/vimfung/luascriptcore/LuaValue;)Lcn/vimfung/luascriptcore/LuaValue;");

    static jclass luaValueClass = LuaJavaType::luaValueClass(env);

    jstring jMethodName = env -> NewStringUTF(methodName.c_str());

    //参数
    jobjectArray argumentArr = env -> NewObjectArray(args.size(), luaValueClass, NULL);
    int index = 0;
    for (LuaArgumentList::iterator it = args.begin(); it != args.end(); it ++)
    {
        LuaValue *argument = *it;
        jobject jArgument = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, argument);
        env -> SetObjectArrayElement(argumentArr, index, jArgument);
        env -> DeleteLocalRef(jArgument);
        index++;
    }

    jobject result = env -> CallStaticObjectMethod(classImportCls, invokeMethodID, moduleClass, jInstance, jMethodName, argumentArr);
    retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, result);
    env -> DeleteLocalRef(result);

    env -> DeleteLocalRef(jMethodName);
    env -> DeleteLocalRef(argumentArr);
    env -> DeleteLocalRef(classImportCls);

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

/**
 * 实例字段获取器调用
 *
 * @param classDescriptor 类描述
 * @param instance 实例对象
 * @param fieldName 字段名称
 *
 * @return 字段值
 */
static LuaValue* _instanceFieldGetterInvokeHandler (LuaContext *context, LuaObjectDescriptor *classDescriptor, LuaUserdataRef instance, std::string fieldName)
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    LuaValue *retValue = NULL;

    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    jclass cls = (jclass)classDescriptor -> getObject();

    //转换为Java的实例对象
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();

    jmethodID getFieldId = env -> GetStaticMethodID(classImportCls, "_getField", "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/String;)Lcn/vimfung/luascriptcore/LuaValue;");

    jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());
    jobject retObj = env -> CallStaticObjectMethod(classImportCls, getFieldId, cls, jInstance, fieldNameStr);

    if (retObj != NULL)
    {
        retValue = LuaJavaConverter::convertToLuaValueByJLuaValue(env, context, retObj);
        env -> DeleteLocalRef(retObj);
    }

    env -> DeleteLocalRef(fieldNameStr);
    env -> DeleteLocalRef(classImportCls);

    LuaJavaEnv::resetEnv(env);

    if (retValue == NULL)
    {
        retValue = new LuaValue();
    }

    return retValue;
}

/**
 * 实例字段设置器调用
 *
 * @param classDescriptor 类描述
 * @param instance 实例对象
 * @param fieldName 字段名字
 * @param value 字段值
 */
static void _instanceFieldSetterInvokeHandler (LuaContext *context, LuaObjectDescriptor *classDescriptor, LuaUserdataRef instance, std::string fieldName, LuaValue *value)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    jclass classImportCls = env -> FindClass("cn/vimfung/luascriptcore/modules/oo/LuaClassImport");
    jclass cls = (jclass)classDescriptor -> getObject();

    //转换为Java的实例对象
    jobject jInstance = (jobject)((LuaObjectDescriptor *)instance -> value) -> getObject();
    jmethodID setFieldId = env -> GetStaticMethodID(classImportCls, "_setField", "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/String;Lcn/vimfung/luascriptcore/LuaValue;)V");
    jstring fieldNameStr = env -> NewStringUTF(fieldName.c_str());

    jobject jValue = LuaJavaConverter::convertToJavaLuaValueByLuaValue(env, context, value);
    env -> CallStaticVoidMethod(classImportCls, setFieldId, cls, jInstance, fieldNameStr, jValue);
    env -> DeleteLocalRef(jValue);

    env -> DeleteLocalRef(fieldNameStr);

    env -> DeleteLocalRef(classImportCls);

    LuaJavaEnv::resetEnv(env);
}


LuaJavaClassImport::LuaJavaClassImport()
    : LuaClassImport ()
{
    onAllowExportsClass(_allowExportsClassHandler);
    onExportsClass(_exportClassHandler);
    onCreateInstance(_createInstanceHandler);
    onClassMethodInvoke(_classMethodInvokeHandler);
    onInstanceMethodInvoke(_instanceMethodInvokeHandler);
    onInstanceFieldGetterInvoke(_instanceFieldGetterInvokeHandler);
    onInstanceFieldSetterInvoke(_instanceFieldSetterInvokeHandler);
}

void LuaJavaClassImport::onRegister(const std::string &name,
                                    LuaContext *context)
{
    //添加对象过滤器
    LuaObjectDescriptor::addPushFilter(_javaObjectPushFilter);
    LuaClassImport::onRegister(name, context);
}

void LuaJavaClassImport::setExportClassList(const std::list<jclass> &exportClassList)
{
    //释放之前的对象
    JNIEnv *env = LuaJavaEnv::getEnv();
    for (std::list<jclass>::iterator it = _exportClassList.begin(); it != _exportClassList.end() ; ++it)
    {
        env -> DeleteGlobalRef(*it);
    }
    LuaJavaEnv::resetEnv(env);

    _exportClassList = exportClassList;
}