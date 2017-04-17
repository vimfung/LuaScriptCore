//
//  LuaUnityDefined.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityDefined_h
#define LuaUnityDefined_h

#include <map>
#include <string>

#if defined (__cplusplus)
extern "C" {
#endif

    /**
     设置本地对象ID处理器
     */
    typedef void (*LuaSetNativeObjectIdHandlerPtr) (long long object, int nativeObjectId, const char * luaObjectId);
    
    /**
     根据实例获取类型名称
     
     @return 类型名称
     */
    typedef char* (*LuaGetClassNameByInstanceHandlerPtr) (const void *object);

    /**
     Lua方法处理器
     */
    typedef void* (*LuaMethodHandlerPtr)(int, const char *, const void *, int);

    /**
     Lua模块方法处理器
     */
    typedef void* (*LuaModuleMethodHandlerPtr) (int moduleId, const char *methodName, const void *argumentsBuffer, int bufferSize);

    /**
     Lua实例创建处理器
     */
    typedef long long (*LuaInstanceCreateHandlerPtr) (int moduleId);

    /**
     Lua实例销毁处理器
     */
    typedef void (*LuaInstanceDestoryHandlerPtr) (long long instance);
    
    /**
     Lua实例描述处理器
     */
    typedef char* (*LuaInstanceDescriptionHandlerPtr) (long long instance);
    
    /**
     Lua实例方法处理器
     */
    typedef void* (*LuaInstanceMethodHandlerPtr) (int classId,  long long instance, const char *methodName, const void *argumentsBuffer, int bufferSize);

    /**
     Lua实例字段获取器
     */
    typedef void* (*LuaInstanceFieldGetterHandlerPtr) (int classId, long long instance, const char *fieldName);

    /**
     Lua实例字段设置处理器
     */
    typedef void* (*LuaInstanceFieldSetterHandlerPtr) (int classId, long long instance, const char *fieldName, const void *valueBuffer, int bufferSize);

    /**
     Lua异常处理器
     */
    typedef void (*LuaExceptionHandlerPtr) (const void *);
    
    /**
     允许类型导出处理器
     */
    typedef bool (*LuaAllowExportsClassHandlerPtr) (int contextId, const char *className);
    
    /**
     获取所有导出类方法
     */
    typedef void* (*LuaAllExportClassMethodHandlerPtr) (int contextId, const char *className);
    
    /**
     获取所有导出实例方法
     */
    typedef void* (*LuaAllExportInstanceMethodHandlerPtr) (int contextId, const char *className);
    
    /**
     获取所有导出字段Getter方法
     */
    typedef void* (*LuaAllExportFieldGetterHandlerPtr) (int contextId, const char *className);
    
    /**
     获取所有导出字段Setter方法
     */
    typedef void* (*LuaAllExportFieldSetterHandlerPtr) (int contextId, const char *className);
    
    /**
     创建原生对象实例
     */
    typedef long long (*LuaCreateNativeObjectHandlerPtr) (int contextId, const char *className);
    
    /**
     * 类方法调用处理器
     */
    typedef void* (*LuaNativeClassMethodInvokeHandlerPtr) (
        int contextId,
        const char *className,
        const char *methodName,
        const void *argumentsBuffer,
        int bufferSize);
    
    /**
     * 实例方法调用处理器
     */
    typedef void* (*LuaNativeInstanceMethodInvokeHandlerPtr) (
        int contextId,
        const char *className,
        long long instance,
        const char *methodName,
        const void *argumentsBuffer,
        int bufferSize);
    
    /**
     Lua实例字段获取器
     */
    typedef void* (*LuaNativeFieldGetterHandlerPtr) (
        int contextId,
        const char *className,
        long long instance,
        const char *fieldName);
    
    /**
     Lua实例字段设置处理器
     */
    typedef void (*LuaNativeFieldSetterHandlerPtr) (
        int contextId,
        const char *className,
        long long instance,
        const char *fieldName,
        const void *valueBuffer,
        int bufferSize);
    
    
    
    typedef std::map<std::string, LuaMethodHandlerPtr> LuaMethodPtrMap;
    typedef std::map<int, LuaExceptionHandlerPtr> LuaContextExceptionPtrMap;
    typedef std::map<int, LuaMethodPtrMap> LuaContextMethodPtrMap;

    
#if defined (__cplusplus)
}
#endif

#endif /* LuaUnityDefined_h */
