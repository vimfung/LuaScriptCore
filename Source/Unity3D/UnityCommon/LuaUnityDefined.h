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
     导出原生类型处理器
     */
    typedef void (*LuaExportsNativeTypeHandlerPtr) (int contextId, const char *typeName);

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
     Lua实例创建处理器
     */
    typedef long long (*LuaInstanceCreateHandlerPtr) (int contextId, int moduleId, const void *argumentsBuffer, int bufferSize);

    /**
     Lua实例销毁处理器
     */
    typedef void (*LuaInstanceDestoryHandlerPtr) (long long instance);
    
    /**
     Lua实例描述处理器
     */
    typedef char* (*LuaInstanceDescriptionHandlerPtr) (long long instance);
    
    /**
     Lua类方法处理器
     */
    typedef void* (*LuaModuleMethodHandlerPtr) (int contextId, int moduleId, const char *methodName, const void *argumentsBuffer, int bufferSize);
    
    /**
     Lua实例方法处理器
     */
    typedef void* (*LuaInstanceMethodHandlerPtr) (int contextId, int classId,  long long instance, const char *methodName, const void *argumentsBuffer, int bufferSize);

    /**
     Lua实例字段获取器
     */
    typedef void* (*LuaInstanceFieldGetterHandlerPtr) (int contextId, int classId, long long instance, const char *fieldName);

    /**
     Lua实例字段设置处理器
     */
    typedef void* (*LuaInstanceFieldSetterHandlerPtr) (int contextId, int classId, long long instance, const char *fieldName, const void *valueBuffer, int bufferSize);

    /**
     Lua异常处理器
     */
    typedef void (*LuaExceptionHandlerPtr) (int contextId, const void *);
    
    typedef std::map<std::string, LuaMethodHandlerPtr> LuaMethodPtrMap;
    typedef std::map<int, LuaExceptionHandlerPtr> LuaContextExceptionPtrMap;
    typedef std::map<int, LuaMethodPtrMap> LuaContextMethodPtrMap;

    
#if defined (__cplusplus)
}
#endif

#endif /* LuaUnityDefined_h */
