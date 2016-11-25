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

#if defined (__cplusplus)
extern "C" {
#endif
    
    using namespace cn::vimfung::luascriptcore;
    
    typedef std::map<std::string, LuaMethodHandlerPtr> LuaMethodPtrMap;
    typedef std::map<int, LuaExceptionHandlerPtr> LuaContextExceptionPtrMap;
    typedef std::map<int, LuaMethodPtrMap> LuaContextMethodPtrMap;
    
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
            LuaObjectEncoder *encoder = new LuaObjectEncoder();
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
                LuaObjectDecoder *decoder = new LuaObjectDecoder(returnBuffer);
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
     创建Lua上下文对象

     @return Lua上下文对象
     */
    int createLuaContext()
    {
        LuaObject::setMappingClassType(typeid(LuaValue).name(), "cn.vimfung.luascriptcore.LuaValue");
        
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
            int bufSize = LuaObjectEncoder::encodeObject(value, result);
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
            int bufSize = LuaObjectEncoder::encodeObject(value, result);
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
                LuaObjectDecoder *decoder = new LuaObjectDecoder(params);
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
            
            LuaValue *value = context -> callMethod(methodName, &args);
            
            int bufSize = LuaObjectEncoder::encodeObject(value, result);
            value -> release();
            
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
    
#if defined (__cplusplus)
}
#endif
