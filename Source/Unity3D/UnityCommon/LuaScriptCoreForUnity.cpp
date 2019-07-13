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
#include <ctype.h>
#include <vector>
#include <stdlib.h>
#include "LuaContext.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectManager.h"
#include "LuaObjectDecoder.hpp"
#include "lunity.h"
#include "LuaUnityEnv.hpp"
#include "LuaFunction.h"
#include "LuaPointer.h"
#include "LuaTuple.h"
#include "LuaExportsTypeManager.hpp"
#include "LuaUnityExportTypeDescriptor.hpp"
#include "LuaUnityExportMethodDescriptor.hpp"
#include "LuaUnityExportPropertyDescriptor.hpp"
#include "LuaValue.h"
#include "LuaTmpValue.hpp"
#include "LuaObjectDescriptor.h"
#include "StringUtils.h"
#include "LuaScriptController.h"

#if defined (__cplusplus)
extern "C" {
#endif
    
    using namespace cn::vimfung::luascriptcore;
        
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
    static LuaValue* luaMethodHandler(LuaContext *context, std::string const& methodName, LuaArgumentList arguments)
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
     导出原生类型处理器

     @param context 上下文对象
     @param typeName 类型名称
     */
    static void luaExportsNativeTypeHandler(LuaContext *context, std::string const& typeName)
    {
        if (!typeName.empty() && typeName[0] != '_')
        {
            LuaUnityEnv::sharedInstance() -> exportsNativeType(context -> objectId(), typeName);
        }
    }
    
    /**
     lua异常处理器

     @param context 上下文对象
     @param errMessage 错误信息
     */
    static void luaExceptionHandler(LuaContext *context, std::string const& errMessage)
    {
        LuaExceptionHandlerPtr handlerPtr = _luaExceptionPtrMap[context -> objectId()];
        if (handlerPtr != NULL)
        {
            handlerPtr (context -> objectId(), errMessage.c_str());
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
     绑定导出原生类型处理

     @param handler 处理器
     */
    void bindExportsNativeTypeHandler(LuaExportsNativeTypeHandlerPtr handler)
    {
        LuaUnityEnv::sharedInstance() -> bindExportsNativeTypeHandler(handler);
    }
    
    /**
     创建Lua上下文对象

     @return Lua上下文对象
     */
    int createLuaContext()
    {
        //设置映射类型
        LuaObjectEncoder::setMappingClassType(typeid(LuaValue).name(), "cn.vimfung.luascriptcore.LuaValue");
        //让LuaTmpValue与LuaValue走一样的序列化操作
        LuaObjectEncoder::setMappingClassType(typeid(LuaTmpValue).name(), "cn.vimfung.luascriptcore.LuaValue");
        LuaObjectEncoder::setMappingClassType(typeid(LuaObjectDescriptor).name(), "cn.vimfung.luascriptcore.LuaObjectDescriptor");
        LuaObjectEncoder::setMappingClassType(typeid(LuaFunction).name(), "cn.vimfung.luascriptcore.LuaFunction");
        LuaObjectEncoder::setMappingClassType(typeid(LuaPointer).name(), "cn.vimfung.luascriptcore.LuaPointer");
        LuaObjectEncoder::setMappingClassType(typeid(LuaTuple).name(), "cn.vimfung.luascriptcore.LuaTuple");
        
        LuaContext *context = new LuaContext("unity3d");
        
        //设置导出原生类型处理器
        context -> onExportsNativeType(luaExportsNativeTypeHandler);
        
        LuaObjectManager::SharedInstance() -> putObject(context);
        context -> release();

        return context -> objectId();
    }
    
    int createLuaScriptController()
    {
        LuaScriptController *controller = new LuaScriptController();
        LuaObjectManager::SharedInstance() -> putObject(controller);
        controller -> release();
        
        return controller -> objectId();
    }
    
    void scriptControllerSetTimeout(int scriptControllerId, int timeout)
    {
        LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
        if (scriptController != NULL)
        {
            scriptController -> setTimeout(timeout);
        }
    }
    
    void scriptControllerForceExit(int scriptControllerId)
    {
        LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
        if (scriptController != NULL)
        {
            scriptController -> forceExit();
        }
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
    
    void raiseException(int nativeContextId, const char *message)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL && message != NULL)
        {
            context -> raiseException(message);
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
    int evalScript(int nativeContextId, const char* script, int scriptControllerId, const void** result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
            
            LuaValue *value = context -> evalScript(script, scriptController);
            
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
    int evalScriptFromFile(int nativeContextId, const char* filePath, int scriptControllerId, const void** result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
            
            LuaValue *value = context -> evalScriptFromFile(filePath, scriptController);
            
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
    
    void retainValue(int nativeContextId, const void *value)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, value);
            
            LuaValue *value =  (LuaValue *)decoder -> readObject();
            context -> retainValue(value);
            value -> release();
            
            decoder -> release();
        }
    }
    
    void releaseValue(int nativeContextId, const void *value)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, value);
            
            LuaValue *value =  (LuaValue *)decoder -> readObject();
            context -> releaseValue(value);
            value -> release();
            
            decoder -> release();
        }
    }
    
    
    /**
     调用方法

     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param params 参数列表
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    int callMethod(int nativeContextId, const char* methodName, const void *params, int scriptControllerId, const void** result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL)
        {
            LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
            
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
            
            LuaValue *retValue = context -> callMethod(methodName, &args, scriptController);
            
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
    int invokeLuaFunction(int nativeContextId, const void* function, const void *params, int scriptControllerId, const void **result)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL && function != NULL)
        {
            LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
            
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
            
            LuaValue *retValue = func -> invoke(&args, scriptController);
            
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
    
    int registerType(int nativeContextId,
                     const char *alias,
                     const char *typeName,
                     const char *parentTypeName,
                     const void *exportsPropertyNames,
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
        using namespace cn::vimfung::luascriptcore;
        
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        LuaUnityExportTypeDescriptor *typeDescriptor = NULL;
        
        if (context != NULL)
        {
            LuaExportTypeDescriptor *parentTypeDescriptor = NULL;
            if (parentTypeName != NULL)
            {
                parentTypeDescriptor = (LuaUnityExportTypeDescriptor *)context -> getExportsTypeManager() -> getExportTypeDescriptor(parentTypeName);
            }
            
            if (parentTypeDescriptor == NULL)
            {
                //Object为基类
                parentTypeDescriptor = (LuaUnityExportTypeDescriptor *)context -> getExportsTypeManager() -> getExportTypeDescriptor("Object");
            }
            
            typeDescriptor = new LuaUnityExportTypeDescriptor(typeName, parentTypeDescriptor);
            
            //设置类型名称映射
            if (typeDescriptor -> typeName() != typeName)
            {
                context -> getExportsTypeManager() -> _mappingType("unity3d", typeName, typeDescriptor -> typeName());
            }
            
            if (alias != NULL && typeDescriptor -> typeName() != alias)
            {
                //如果传入格式不等于导出类型名称，则进行映射操作
                context -> getExportsTypeManager() -> _mappingType("unity3d", typeName, alias);
            }
            
            typeDescriptor -> createInstanceHandler = instanceCreateHandler;
            typeDescriptor -> destroyInstanceHandler = instanceDestroyHandler;
            typeDescriptor -> instanceDescriptionHandler = instanceDescriptionHandler;
            
            if (exportsClassMethodNames != NULL)
            {
                //注册类方法
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsClassMethodNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string methodName = decoder -> readString();
                    //分割方法组成部分
                    std::deque<std::string> methodComps = StringUtils::split(methodName, "_", false);
                    LuaUnityExportMethodDescriptor *methodDescriptor = new LuaUnityExportMethodDescriptor(methodComps[0], methodComps[1], classMethodRouteHandler);
                    typeDescriptor -> addClassMethod(methodComps[0], methodDescriptor);
                    methodDescriptor -> release();
                }
                decoder -> release();
            }
            
            if (exportsInstanceMethodNames != NULL)
            {
                //注册实例方法
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsInstanceMethodNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string methodName = decoder -> readString();
                    //分割方法组成部分
                    std::deque<std::string> methodComps = StringUtils::split(methodName, "_", false);
                    LuaUnityExportMethodDescriptor *methodDescriptor = new LuaUnityExportMethodDescriptor(methodComps[0], methodComps[1],  instanceMethodRouteHandler);
                    typeDescriptor -> addInstanceMethod(methodComps[0], methodDescriptor);
                    methodDescriptor -> release();
                }
                decoder -> release();
            }
            
            if (exportsPropertyNames != NULL)
            {
                //注册属性
                LuaObjectDecoder *decoder = new LuaObjectDecoder(context, exportsPropertyNames);
                int size = decoder -> readInt32();
                for (int i = 0; i < size; i++)
                {
                    std::string fieldName = decoder -> readString();
                    std::deque<std::string> fieldNameComps = StringUtils::split(fieldName, "_", false);
                    
                    bool canRead = false;
                    bool canWrite = false;
                    if (fieldNameComps[1] == "r")
                    {
                        canRead = true;
                    }
                    else if (fieldNameComps[1] == "w")
                    {
                        canWrite = true;
                    }
                    else
                    {
                        canRead = true;
                        canWrite = true;
                    }
                    
                    LuaUnityExportPropertyDescriptor *propertyDescriptor = new LuaUnityExportPropertyDescriptor(fieldNameComps[0], canRead, canWrite, instanceFieldGetterRouteHandler, instanceFieldSetterRouteHandler);
                    typeDescriptor -> addProperty(propertyDescriptor -> name(), propertyDescriptor);
                    propertyDescriptor -> release();
                }
                decoder -> release();
            }

            context -> getExportsTypeManager() -> exportsType(typeDescriptor);
            
            typeDescriptor -> release();
        }
        
        return typeDescriptor != NULL ? typeDescriptor -> objectId() : -1;
    }
    
    void runThread(int nativeContextId,
                   const void* function,
                   const void *params,
                   int scriptControllerId)
    {
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(nativeContextId));
        if (context != NULL && function != NULL)
        {
            LuaScriptController *scriptController = dynamic_cast<LuaScriptController *>(LuaObjectManager::SharedInstance() -> getObject(scriptControllerId));
            
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
            
            context -> runThread(func, args, scriptController);
            
            //释放参数内存
            for (LuaArgumentList::iterator it = args.begin(); it != args.end(); ++it)
            {
                LuaValue *value = *it;
                value -> release();
            }
            
            //释放方法
            func -> release();
        }
    }
    
    extern int tableSetObject(int contextId,
                              const void *valueData,
                              const char *keyPath,
                              const void *object,
                              const void **result)
    {
        
        LuaContext *context = dynamic_cast<LuaContext *>(LuaObjectManager::SharedInstance() -> getObject(contextId));
        if (context != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, valueData);
            LuaValue *value =  (LuaValue *)decoder -> readObject();
            decoder -> release();
            
            LuaObjectDecoder *objDecoder = new LuaObjectDecoder(context, object);
            LuaValue *objValue = dynamic_cast<LuaValue *>(objDecoder -> readObject());
            objDecoder -> release();
            
            value -> setObject(keyPath, objValue, context);
            
            int bufSize = LuaObjectEncoder::encodeObject(context, value, result);
            
            objValue -> release();
            value -> release();
            
            return bufSize;
        }
        
        return 0;
    }
    
#if defined (__cplusplus)
}
#endif
