//
//  LuaScriptCore.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/11.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef LuaScriptCore_h
#define LuaScriptCore_h

#if defined (__cplusplus)
extern "C" {
#endif
    
    /**
     Lua方法处理器
     */
    typedef void* (*LuaMethodHandlerPtr)(int, const char *, const void *, int);
    
    /**
     Lua异常处理器
     */
    typedef void (*LuaExceptionHandlerPtr) (const void *);
    
    /**
     创建Lua上下文对象

     @return 上下文对象标识
     */
    extern int createLuaContext();
    
    /**
     释放对象
     
     @param objectId 对象ID
     */
    extern void releaseObject(int objectId);
    
    /**
     添加Lua的搜索路径
     
     @param nativeContextId 本地上下文对象ID
     @param path 路径
     */
    extern void addSearchPath(int nativeContextId, const char *path);
    
    /**
     设置异常处理器

     @param nativeContextId 本地上下文对象ID
     @param handler 处理器
     */
    extern void setExceptionHandler (int nativeContextId, LuaExceptionHandlerPtr handler);
    
    /**
     解析Lua脚本
     
     @param nativeContextId 本地上下文对象ID
     @param script Lua脚本
     @param result 返回值(输出参数)
     
     @return 返回值的缓冲区大小
     */
    extern int evalScript(int nativeContextId, const char* script, const void** result);
    
    
    /**
     从文件中解析Lua脚本

     @param nativeContextId 本地上下文对象ID
     @param filePath Lua文件路径
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    extern int evalScriptFromFile(int nativeContextId, const char* filePath, const void** result);
    
    /**
     调用Lua方法

     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param params 参数列表
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    extern int callMethod(int nativeContextId, const char* methodName, const void *params, const void** result);
    
    
    /**
     注册Lua方法

     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param methodPtr 方法处理器指针
     */
    extern void registerMethod(int nativeContextId, const char* methodName, LuaMethodHandlerPtr methodPtr);
    
#if defined (__cplusplus)
}
#endif


#endif /* LuaScriptCore_h */
