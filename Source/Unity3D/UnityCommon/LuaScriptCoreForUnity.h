//
//  LuaScriptCore.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/11.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef LuaScriptCore_h
#define LuaScriptCore_h

#if _WINDOWS

#define LuaScriptCoreApi __declspec(dllexport)

#else

#define LuaScriptCoreApi

#endif

#include "LuaUnityDefined.h"

#if defined (__cplusplus)
extern "C" {
#endif
    
    /**
     绑定设置原生对象标识方法

     @param handler 处理器
     */
    LuaScriptCoreApi extern void bindSetNativeObjectIdHandler(LuaSetNativeObjectIdHandlerPtr handler);
    
    
    /**
     绑定根据实例获取类型名称方法

     @param handler 处理器
     */
    LuaScriptCoreApi extern void bindGetClassNameByInstanceHandler (LuaGetClassNameByInstanceHandlerPtr handler);
    
    /**
     创建Lua上下文对象

     @return 上下文对象标识
     */
	LuaScriptCoreApi extern int createLuaContext();
    
    /**
     释放对象
     
     @param objectId 对象ID
     */
	LuaScriptCoreApi extern void releaseObject(int objectId);
    
    /**
     添加Lua的搜索路径
     
     @param nativeContextId 本地上下文对象ID
     @param path 路径
     */
	LuaScriptCoreApi extern void addSearchPath(int nativeContextId, const char *path);
    
    /**
     设置异常处理器

     @param nativeContextId 本地上下文对象ID
     @param handler 处理器
     */
	LuaScriptCoreApi extern void setExceptionHandler (int nativeContextId, LuaExceptionHandlerPtr handler);
    
    /**
     设置全局变量

     @param nativeContextId 本地上下文对象ID
     @param name 变量名称
     @param value 变量值
     */
    LuaScriptCoreApi extern void setGlobal(int nativeContextId, const char *name, const void *value);
    
    /**
     获取全局变量

     @param nativeContextId 本地上下文对象ID
     @param name 变量名称
     @param result 变量值缓存
     @return 变量值缓存长度
     */
    LuaScriptCoreApi extern int getGlobal(int nativeContextId, const char *name, const void **result);
    
    /**
     解析Lua脚本
     
     @param nativeContextId 本地上下文对象ID
     @param script Lua脚本
     @param result 返回值(输出参数)
     
     @return 返回值的缓冲区大小
     */
	LuaScriptCoreApi extern int evalScript(int nativeContextId, const char *script, const void **result);
    
    
    /**
     从文件中解析Lua脚本

     @param nativeContextId 本地上下文对象ID
     @param filePath Lua文件路径
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
	LuaScriptCoreApi extern int evalScriptFromFile(int nativeContextId, const char *filePath, const void **result);
    
    /**
     调用Lua方法

     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param params 参数列表
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
	LuaScriptCoreApi extern int callMethod(int nativeContextId, const char* methodName, const void *params, const void **result);
    
    /**
     调用Lua方法
     
     @param nativeContextId 本地上下文对象ID
     @param function 方法
     @param params 参数列表
     @param result 返回值（输出参数）
     
     @return 返回值的缓冲区大小
     */
    LuaScriptCoreApi extern int invokeLuaFunction(int nativeContextId, const void* function, const void *params, const void **result);
    
    
    /**
     注册Lua方法

     @param nativeContextId 本地上下文对象ID
     @param methodName 方法名称
     @param methodPtr 方法处理器指针
     */
	LuaScriptCoreApi extern void registerMethod(int nativeContextId, const char *methodName, LuaMethodHandlerPtr methodPtr);
    
    
    /**
     注册模块

     @param nativeContextId 本地上下文对象ID
     @param moduleName 模块名称
     @param exportsMethodNames 导出方法名称列表
     @param methodRouteHandler 方法路由处理回调
     
     @return 模块的本地标识 
     */
    LuaScriptCoreApi extern int registerModule(int nativeContextId, const char *moduleName, const void *exportsMethodNames, LuaModuleMethodHandlerPtr methodRouteHandler);
    
    
    /**
     判断模块是否注册

     @param nativeContextId 本地上下文对象ID
     @param moduleName 模块名称
     @return true 表示已注册，false 表示尚未注册。
     */
    LuaScriptCoreApi extern bool isModuleRegisted(int nativeContextId, const char *moduleName);
    
    
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
     @param instanceDescriptionHandler 类型描述处理器回调
     @param instanceFieldGetterRouteHandler 实例字段获取器路由处理回调
     @param instanceFieldSetterRouteHandler 实例字段设置器路由回调
     @param instanceMethodRouteHandler 实例方法路由处理回调
     @param classMethodRouteHandler 类方法路由处理回调
     @return 类的本地标识
     */
    LuaScriptCoreApi extern int registerClass(int nativeContextId,
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
                                              LuaModuleMethodHandlerPtr classMethodRouteHandler);
    
    /**
     注册导出类型

     @param nativeContextId 上下文标识
     @param className 类名
     @param allowExportsClassHandler 是否允许导出类型处理器
     @param allExportClassMethods 导出所有类方法
     @param allExportInstanceMethods 导出所有实例方法
     @param allExportGetterFields 导出所有字段的Getter
     @param allExportSetterFields 导出所有字段的Setter
     @param instanceCreateHandler 实例对象实例方法
     @param classMethodInvokeHandler 类方法调用处理
     @param instanceMethodInvokeHandler 实例方法调用
     @param fieldGetterHandler 字段获取器
     @param fieldSetterHandler 字段设置器
     */
    LuaScriptCoreApi extern void registerClassImport(int nativeContextId,
                                                    const char *className,
                                                    LuaAllowExportsClassHandlerPtr allowExportsClassHandler,
                                                    LuaAllExportClassMethodHandlerPtr allExportClassMethods,
                                                    LuaAllExportInstanceMethodHandlerPtr allExportInstanceMethods,
                                                    LuaAllExportFieldGetterHandlerPtr allExportGetterFields,
                                                    LuaAllExportFieldSetterHandlerPtr allExportSetterFields,
                                                    LuaCreateNativeObjectHandlerPtr instanceCreateHandler,
                                                    LuaNativeClassMethodInvokeHandlerPtr classMethodInvokeHandler,
                                                    LuaNativeInstanceMethodInvokeHandlerPtr instanceMethodInvokeHandler,
                                                    LuaNativeFieldGetterHandlerPtr fieldGetterHandler,
                                                    LuaNativeFieldSetterHandlerPtr fieldSetterHandler);
#if defined (__cplusplus)
}
#endif


#endif /* LuaScriptCore_h */
