//
//  LuaScriptCore.c
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/11.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#include "LuaScriptCoreForUnity.h"
#include <iostream>
#include <typeinfo>
#include "LuaContext.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectManager.h"
#include "LuaObjectDecoder.hpp"
#include "lunity.h"
#include "LuaUnityObjectClass.hpp"
#include "LuaObjectInstanceDescriptor.h"
#include "LuaUnityModule.hpp"
#include "LuaUnityEnv.hpp"
#include "LuaFunction.h"
#include "LuaPointer.h"
#include "LuaTuple.h"
#include "LuaUnityClassImport.hpp"

#if defined (__cplusplus)
extern "C" {
#endif
    
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;
    
    typedef std::map<int, LuaUnityObjectClass *> LuaClassMap;
        
    /**
     方法指针集合
     */
    static LuaContextMethodPtrMap _luaMethodPtrMap;
    
    /**
     异常处理器指针集合
     */
    static LuaContextExceptionPtrMap _luaExceptionPtrMap;
    
    /**
     Lua方法处理器

     @param methodName 方法名称
     @param arguments 参数列表
     @return 返回值
     */
    static LuaValue* luaMethodHandler(LuaContext *context, std::string methodName, LuaArgumentList arguments)
    {
        LuaMethodPtrMap methodPtrMap = _luaMethodPtrMap[context -> objectId()];
        LuaMethodPtrMap::iterator it = methodPtrMap.find(methodName);
        if (it != methodPtrMap.end())
        {
            //找到相关注册方法
            LuaMethodHandlerPtr methodPtr = it -> second;
            
            //编码参数列表
            LuaObjectEncoder *encoder = new LuaObjectEncoder(context);
            encoder -> writeInt32((int)arguments.size());
            
            for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
            {
                LuaValue *value = *it;
                encoder -> writeObject(value);
            }
            
            //paramsBuffer的内由C#端进行释放
            const void *paramsBuffer = encoder -> cloneBuffer();
            void *returnBuffer = methodPtr(context -> objectId(), methodName.c_str(), paramsBuffer, encoder -> getBufferLength());
            
            encoder -> release();
            
            LuaValue *retValue = NULL;
            if (returnBuffer != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, returnBuffer);
                retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
                decoder -> release();
                
                //释放C＃中申请的内存
                free(returnBuffer);
            }
            else
            {
                retValue = LuaValue::NilValue();
            }
            
            return retValue;
        }
        
        return NULL;
    }
    
    
    /**
     Lua模块方法处理器

     @param module 模块对象
     @param methodName 方法名称
     @param arguments 参数列表
     @return 返回值
     */
    static LuaValue* luaModuleMethodHandler(LuaModule *module, std::string methodName, LuaArgumentList arguments)
    {
        LuaUnityModule *unityModule = (LuaUnityModule *)module;
        LuaModuleMethodHandlerPtr methodPtr = unityModule -> getMethodHandler();
        
        if (methodPtr != NULL)
        {
            //编码参数列表
            LuaObjectEncoder *encoder = new LuaObjectEncoder(module -> getContext());
            encoder -> writeInt32((int)arguments.size());
            
            for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
            {
                LuaValue *value = *it;
                encoder -> writeObject(value);
            }
            
            //paramsBuffer的内容由C#端进行释放
            const void *paramsBuffer = encoder -> cloneBuffer();
            void *returnBuffer = methodPtr(module -> objectId(), methodName.c_str(), paramsBuffer, encoder -> getBufferLength());
            
            encoder -> release();
            
            LuaValue *retValue = NULL;
            if (returnBuffer != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(module -> getContext(), returnBuffer);
                retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
                decoder -> release();
                
                //释放C＃中申请的内存
                free(returnBuffer);
            }
            else
            {
                retValue = LuaValue::NilValue();
            }
            
            return retValue;
        }
        
        return NULL;
    }
    
    /**
     Lua类方法处理器
     
     @param module 模块对象
     @param methodName 方法名称
     @param arguments 参数列表
     @return 返回值
     */
    static LuaValue* luaClassMethodHandler(LuaModule *module, std::string methodName, LuaArgumentList arguments)
    {
        LuaUnityObjectClass *unitObjectClass = (LuaUnityObjectClass *)module;
        LuaModuleMethodHandlerPtr methodPtr = unitObjectClass -> getClassMethodHandler();
        if (methodPtr != NULL)
        {
            //编码参数列表
            LuaObjectEncoder *encoder = new LuaObjectEncoder(module -> getContext());
            encoder -> writeInt32((int)arguments.size());
            
            for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
            {
                LuaValue *value = *it;
                encoder -> writeObject(value);
            }
            
            //paramsBuffer的内容由C#端进行释放
            const void *paramsBuffer = encoder -> cloneBuffer();
            void *returnBuffer = methodPtr(module -> objectId(), methodName.c_str(), paramsBuffer, encoder -> getBufferLength());
            
            encoder -> release();
            
            LuaValue *retValue = NULL;
            if (returnBuffer != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(module -> getContext(), returnBuffer);
                retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
                decoder -> release();
                
                //释放C＃中申请的内存
                free(returnBuffer);
            }
            else
            {
                retValue = LuaValue::NilValue();
            }
            
            return retValue;
        }
        
        return NULL;
    }
    
    
    /**
     Lua实例方法处理器

     @param instance 实例
     @param objectClass 对象类型
     @param methodName 方法名称
     @param arguments 参数列表
     @return 返回值
     */
    static LuaValue* luaInstanceMethodHandler (LuaUserdataRef instance, LuaObjectClass *objectClass, std::string methodName, LuaArgumentList arguments)
    {
        LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objectClass;

        LuaInstanceMethodHandlerPtr methodPtr = unityObjectClass -> getInstanceMethodHandler();
        if (methodPtr != NULL)
        {
            //编码参数列表
            LuaObjectEncoder *encoder = new LuaObjectEncoder(unityObjectClass -> getContext());
            encoder -> writeInt32((int)arguments.size());
            
            for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
            {
                LuaValue *value = *it;
                encoder -> writeObject(value);
            }
            
            LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance -> value;
            
            //paramsBuffer的内容由C#端进行释放
            const void *paramsBuffer = encoder -> cloneBuffer();
            void *returnBuffer = methodPtr(objectClass -> objectId(),
                                           (long long) objDesc -> getObject(),
                                           methodName.c_str(),
                                           paramsBuffer,
                                           encoder -> getBufferLength());
            
            encoder -> release();
            
            LuaValue *retValue = NULL;
            if (returnBuffer != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(unityObjectClass -> getContext(), returnBuffer);
                retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
                decoder -> release();
                
                //释放C＃中申请的内存
                free(returnBuffer);
            }
            else
            {
                retValue = LuaValue::NilValue();
            }
            
            return retValue;
        }
        
        return NULL;
    }
    
    
    /**
     实例字段获取器

     @param instance 实例
     @param objectClass 对象类型
     @param fieldName 字段名
     @return 字段值
     */
    static LuaValue* luaInstanceFieldGetterHandler (LuaUserdataRef instance, LuaObjectClass *objectClass, std::string fieldName)
    {
        LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objectClass;
        
        LuaInstanceFieldGetterHandlerPtr methodHandler = unityObjectClass -> getFieldGetterHandler();
        if (methodHandler != NULL)
        {
            LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance -> value;
            void *returnBuffer = methodHandler (unityObjectClass -> objectId(), (long long) objDesc -> getObject(), fieldName.c_str());
            
            LuaValue *retValue = NULL;
            if (returnBuffer != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(unityObjectClass -> getContext(), returnBuffer);
                retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
                decoder -> release();
                
                //释放C＃中申请的内存
                free(returnBuffer);
            }
            else
            {
                retValue = LuaValue::NilValue();
            }
            
            return retValue;
        }
        
        return NULL;
    }
    
    
    /**
     实例字段获取器

     @param instance 实例
     @param objectClass 对象类型
     @param fieldName 字段名
     @param value 字段值
     */
    static void luaInstanceFieldSetterHandler (LuaUserdataRef instance, LuaObjectClass *objectClass, std::string fieldName, LuaValue *value)
    {
        using namespace cn::vimfung::luascriptcore::modules::oo;
        
        LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objectClass;
        
        LuaInstanceFieldSetterHandlerPtr methodHandler = unityObjectClass -> getFieldSetterHandler();
        if (methodHandler != NULL)
        {
            LuaObjectEncoder *encoder = new LuaObjectEncoder(unityObjectClass -> getContext());
            if (value != NULL)
            {
                encoder -> writeObject(value);
            }
            else
            {
                LuaValue *nilValue = LuaValue::NilValue();
                encoder -> writeObject(nilValue);
                nilValue -> release();
            }
            
            //valueBuf的内容由C#端进行释放
            const void *valueBuf = encoder -> cloneBuffer();
            
            LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance -> value;
            methodHandler (unityObjectClass -> objectId(),
                           (long long) objDesc -> getObject(),
                           fieldName.c_str(),
                           valueBuf,
                           encoder -> getBufferLength());
            
            encoder -> release();
        }
    }
    
    
    /**
     lua异常处理器

     @param context 上下文对象
     @param errMessage 错误信息
     */
    static void luaExceptionHandler(LuaContext *context, std::string errMessage)
    {
        LuaExceptionHandlerPtr handlerPtr = _luaExceptionPtrMap[context -> objectId()];
        if (handlerPtr != NULL)
        {
            handlerPtr (errMessage.c_str());
        }
    }
    
    /**
     绑定设置原生对象标识方法
     
     @param handler 处理器
     */
    void bindSetNativeObjectIdHandler(LuaSetNativeObjectIdHandlerPtr handler)
    {
        LuaUnityEnv::sharedInstance() -> bindSetNativeObjectIdHandler(handler);
    }
    
    /**
     绑定根据实例获取类型名称方法
     
     @param handler 处理器
     */
    void bindGetClassNameByInstanceHandler (LuaGetClassNameByInstanceHandlerPtr handler)
    {
        LuaUnityEnv::sharedInstance() -> bindGetClassNameByInstanceHandler(handler);
    }
    
    /**
     创建Lua上下文对象

     @return Lua上下文对象
     */
    int createLuaContext()
    {
        //设置映射类型
        LuaObjectEncoder::setMappingClassType(typeid(LuaValue).name(), "cn.vimfung.luascriptcore.LuaValue");
        LuaObjectEncoder::setMappingClassType(typeid(LuaObjectDescriptor).name(), "cn.vimfung.luascriptcore.LuaObjectDescriptor");
        LuaObjectEncoder::setMappingClassType(typeid(LuaObjectInstanceDescriptor).name(), "cn.vimfung.luascriptcore.modules.oo.LuaObjectInstanceDescriptor");
        LuaObjectEncoder::setMappingClassType(typeid(LuaFunction).name(), "cn.vimfung.luascriptcore.LuaFunction");
        LuaObjectEncoder::setMappingClassType(typeid(LuaPointer).name(), "cn.vimfung.luascriptcore.LuaPointer");
        LuaObjectEncoder::setMappingClassType(typeid(LuaTuple).name(), "cn.vimfung.luascriptcore.LuaTuple");
        
        LuaContext *context = new LuaContext();
        LuaObjectManager::SharedInstance() -> putObject(context);
        context -> release();

        return context -> objectId();
    }
    
    /**
     添加Lua的搜索路径
     
     @param nativeContextId 本地上下文对象ID
     @param path 路径
     */
    void addSearchPath(int nativeContextId, const char *path)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            context -> addSearchPath(path);
        }
    }
    
    /**
     设置异常处理器
     
     @param nativeContextId 本地上下文对象ID
     @param handler 处理器
     */
    void setExceptionHandler (int nativeContextId, LuaExceptionHandlerPtr handler)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            _luaExceptionPtrMap[nativeContextId] = handler;
            context -> onException(luaExceptionHandler);
        }
    }
    
    /**
     删除Lua上下文对象

     @param objectId 本地上下文对象ID
     */
    void releaseObject(int objectId)
    {
        LuaObjectManager::SharedInstance() -> removeObject(objectId);
    }
    
    /**
     解析Lua脚本

     @param nativeContextId 本地上下文对象ID
     @param script Lua脚本
     @param result 返回值(输出参数)
     
     @return 值对象
     */
    int evalScript(int nativeContextId, const char* script, const void** result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaValue *value = context -> evalScript(script);
            
            LuaObjectManager::SharedInstance() -> putObject(value);
            int bufSize = LuaObjectEncoder::encodeObject(context, value, result);
            
            value -> release();
            
            return bufSize;
        }
        
        return 0;
    }
    
    /**
     从文件中解析Lua脚本
     
     @param nativeContextId 本地上下文对象ID
     @param filePath Lua文件路径
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    int evalScriptFromFile(int nativeContextId, const char* filePath, const void** result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaValue *value = context -> evalScriptFromFile(filePath);
            
            LuaObjectManager::SharedInstance() -> putObject(value);
            int bufSize = LuaObjectEncoder::encodeObject(context, value, result);
            
            value -> release();
            
            return bufSize;
        }
        
        return 0;
    }
    
    void setGlobal(int nativeContextId, const char *name, const void *value)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, value);
            
            LuaValue *value =  (LuaValue *)decoder -> readObject();
            context -> setGlobal(name, value);
            value -> release();
            
            decoder -> release();
        }
    }
    
    int getGlobal(int nativeContextId, const char *name, const void **result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaValue *value = context -> getGlobal(name);
            
            LuaObjectManager::SharedInstance() -> putObject(value);
            int bufSize = LuaObjectEncoder::encodeObject(context, value, result);
            
            value -> release();
            
            return bufSize;
        }
        
        return 0;
    }
    
    
    /**
     调用方法

     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param params 参数列表
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    int callMethod(int nativeContextId, const char* methodName, const void *params, const void** result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaArgumentList args;
            
            if (params != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, params);
                int size = decoder -> readInt32();
                
                for (int i = 0; i < size; i++)
                {
                    LuaValue *value = dynamic_cast<LuaValue *>(decoder -> readObject());
                    if (value != NULL)
                    {
                        args.push_back(value);
                    }
                }
                decoder -> release();

            }
            
            LuaValue *retValue = context -> callMethod(methodName, &args);
            
            LuaObjectManager::SharedInstance() -> putObject(retValue);
            int bufSize = LuaObjectEncoder::encodeObject(context, retValue, result);
            
            retValue -> release();
            
            //释放参数内存
            for (LuaArgumentList::iterator it = args.begin(); it != args.end(); ++it)
            {
                LuaValue *value = *it;
                value -> release();
            }
            
            return bufSize;
        }
        
        return 0;
    }
    
    /**
     调用Lua方法
     
     @param nativeContextId 本地上下文对象ID
     @param function 方法
     @param params 参数列表
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    int invokeLuaFunction(int nativeContextId, const void* function, const void *params, const void **result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL && function != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, function);
            LuaFunction *func = dynamic_cast<LuaFunction *>(decoder -> readObject());
            decoder -> release();
            
            LuaArgumentList args;
            
            if (params != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, params);
                int size = decoder -> readInt32();
                
                for (int i = 0; i < size; i++)
                {
                    LuaValue *value = dynamic_cast<LuaValue *>(decoder -> readObject());
                    if (value != NULL)
                    {
                        args.push_back(value);
                    }
                }
                decoder -> release();
            }
            
            LuaValue *retValue = func -> invoke(&args);
            
            LuaObjectManager::SharedInstance() -> putObject(retValue);
            int bufSize = LuaObjectEncoder::encodeObject(context, retValue, result);
            
            retValue -> release();
            
            //释放参数内存
            for (LuaArgumentList::iterator it = args.begin(); it != args.end(); ++it)
            {
                LuaValue *value = *it;
                value -> release();
            }
            
            //释放方法
            func -> release();
            
            return bufSize;
        }
        
        return 0;
    }
    
    /**
     注册Lua方法
     
     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param methodPtr 方法处理器指针
     */
    void registerMethod(int nativeContextId, const char* methodName, LuaMethodHandlerPtr methodPtr)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            _luaMethodPtrMap[nativeContextId][methodName] = methodPtr;
            context -> registerMethod(methodName, luaMethodHandler);
        }
    }
    
    /**
     注册模块
     
     @param nativeContextId 本地上下文对象ID
     @param moduleName 模块名称
     */
    int registerModule(int nativeContextId, const char *moduleName, const void *exportsMethodNames, LuaModuleMethodHandlerPtr methodRouteHandler)
    {
        LuaUnityModule *module = NULL;
        
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            module = new LuaUnityModule();
            module -> setMethodHandler(methodRouteHandler);
            context -> registerModule(moduleName, module);
            
            if (exportsMethodNames != NULL)
            {
                //注册方法
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsMethodNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string methodName = decoder -> readString();
                    module -> registerMethod(methodName, luaModuleMethodHandler);
                }
                decoder -> release();
            }
            
        }
        
        return module != NULL ? module -> objectId() : -1;
    }
    
    /**
     判断模块是否注册
     
     @param nativeContextId 本地上下文对象ID
     @param moduleName 模块名称
     @return true 表示已注册，false 表示尚未注册。
     */
    bool isModuleRegisted(int nativeContextId, const char *moduleName)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            return context -> isModuleRegisted(moduleName);
        }
        return false;
    }
    
    /**
     注册类型
     
     @param nativeContextId 本地上下文对象ID
     @param className 类名称
     @param superClassName 父类名称
     @param exportsSetterNames 导出Setter名称列表
     @param exportsGetterNames 导出Getter名称列表
     @param exportsInstanceMethodNames 导出实例方法名称列表
     @param exportsClassMethodNames 导出类方法名称列表
     @param instanceCreateHandler 实例创建处理回调
     @param instanceDestroyHandler 实例销毁处理回调
     @param instanceDescriptionHandler 实例描述处理器回调
     @param instanceFieldGetterRouteHandler 实例字段获取器路由处理回调
     @param instanceFieldSetterRouteHandler 实例字段设置器路由回调
     @param instanceMethodRouteHandler 实例方法路由处理回调
     @param classMethodRouteHandler 类方法路由处理回调
     @return 类的本地标识
     */
    int registerClass(int nativeContextId,
                      const char *className,
                      const char *superClassName,
                      const void *exportsSetterNames,
                      const void *exportsGetterNames,
                      const void *exportsInstanceMethodNames,
                      const void *exportsClassMethodNames,
                      LuaInstanceCreateHandlerPtr instanceCreateHandler,
                      LuaInstanceDestoryHandlerPtr instanceDestroyHandler,
                      LuaInstanceDescriptionHandlerPtr instanceDescriptionHandler,
                      LuaInstanceFieldGetterHandlerPtr instanceFieldGetterRouteHandler,
                      LuaInstanceFieldSetterHandlerPtr instanceFieldSetterRouteHandler,
                      LuaInstanceMethodHandlerPtr instanceMethodRouteHandler,
                      LuaModuleMethodHandlerPtr classMethodRouteHandler)
    {
        using namespace cn::vimfung::luascriptcore::modules::oo;
        
        LuaUnityObjectClass *objectClass = NULL;
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        
        if (context != NULL)
        {
            LuaObjectClass *superClass = NULL;
            if (superClassName != NULL)
            {
                superClass = (LuaObjectClass *)context -> getModule(superClassName);
            }
            objectClass = new LuaUnityObjectClass(superClass);
            objectClass -> setClassMethodHandler(classMethodRouteHandler);
            objectClass -> setInstanceMethodHandler(instanceMethodRouteHandler);
            objectClass -> setInstanceDescriptionHandler(instanceDescriptionHandler);
            objectClass -> setFieldGetterHandler(instanceFieldGetterRouteHandler);
            objectClass -> setFieldSetterHandler(instanceFieldSetterRouteHandler);
            objectClass -> setInstanceCreateHandler(instanceCreateHandler);
            objectClass -> setInstanceDestroyHandler(instanceDestroyHandler);
            
            context -> registerModule(className, objectClass);
            
            objectClass -> release();
            
            std::map<std::string, std::string> fields;
            if (exportsGetterNames != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsGetterNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string fieldName = decoder -> readString();
                    fields[fieldName] = "r";
                }
                decoder -> release();
            }
            if (exportsSetterNames != NULL)
            {
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsSetterNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string fieldName = decoder -> readString();
                    std::string value = fields[fieldName];
                    value = value + "w";
                    fields[fieldName] = value;
                }
                decoder -> release();
            }
            
            for (std::map<std::string, std::string>::iterator it = fields.begin(); it != fields.end(); it++)
            {
                std::string fieldName = it -> first;
                std::string access = it -> second;
                
                if (access.empty())
                {
                    continue;
                }
                
                LuaInstanceGetterHandler getterHandler = NULL;
                LuaInstanceSetterHandler setterHandler = NULL;
                
                if (access == "r")
                {
                    getterHandler = luaInstanceFieldGetterHandler;
                }
                else if (access == "w")
                {
                    setterHandler = luaInstanceFieldSetterHandler;
                }
                else
                {
                    getterHandler = luaInstanceFieldGetterHandler;
                    setterHandler = luaInstanceFieldSetterHandler;
                }
                
                objectClass -> registerInstanceField(fieldName, getterHandler, setterHandler);
            }

            if (exportsInstanceMethodNames != NULL)
            {
                //注册实例方法
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsInstanceMethodNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string methodName = decoder -> readString();
                    objectClass -> registerInstanceMethod(methodName, luaInstanceMethodHandler);
                }
                
                decoder -> release();
            }

            if (exportsClassMethodNames != NULL)
            {
                //注册类方法
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsClassMethodNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string methodName = decoder -> readString();
                    objectClass -> registerMethod(methodName, luaClassMethodHandler);
                }
                decoder -> release();
            }
        
        }
        
        return objectClass != NULL ? objectClass -> objectId() : -1;
    }
    
    void registerClassImport(int nativeContextId,
                             const char *className,
                             LuaCheckObjectSubclassHandlerPtr checkObjectSubclassHandler,
                             LuaAllowExportsClassHandlerPtr allowExportsClassHandler,
                             LuaAllExportClassMethodHandlerPtr allExportClassMethods,
                             LuaAllExportInstanceMethodHandlerPtr allExportInstanceMethods,
                             LuaAllExportFieldGetterHandlerPtr allExportGetterFields,
                             LuaAllExportFieldSetterHandlerPtr allExportSetterFields,
                             LuaCreateNativeObjectHandlerPtr instanceCreateHandler,
                             LuaNativeClassMethodInvokeHandlerPtr classMethodInvokeHandler,
                             LuaNativeInstanceMethodInvokeHandlerPtr instanceMethodInvokeHandler,
                             LuaNativeFieldGetterHandlerPtr fieldGetterHandler,
                             LuaNativeFieldSetterHandlerPtr fieldSetterHandler)
    {
        using namespace cn::vimfung::luascriptcore::modules::oo;
        
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaUnityClassImport *classImport = new LuaUnityClassImport();
            
            classImport -> setCheckObjectSubclassHandler(checkObjectSubclassHandler);
            classImport -> setAllowExportsClassHandler(allowExportsClassHandler);
            classImport -> setAllExportClassMethodHandler(allExportClassMethods);
            classImport -> setAllExportInstanceMethodHandler(allExportInstanceMethods);
            classImport -> setAllExportInstanceFieldGetterHandler(allExportGetterFields);
            classImport -> setAllExportInstanceFieldSetterHandler(allExportSetterFields);
            classImport -> setCreateObjectHandler(instanceCreateHandler);
            classImport -> setClassMethodInvokeHandler(classMethodInvokeHandler);
            classImport -> setInstanceMethodInvokeHandler(instanceMethodInvokeHandler);
            classImport -> setFieldGetterHandler(fieldGetterHandler);
            classImport -> setFieldSetterHandler(fieldSetterHandler);
            
            context -> registerModule(className, classImport);
            
            classImport -> release();
        }
    }
    
#if defined (__cplusplus)
}
#endif
