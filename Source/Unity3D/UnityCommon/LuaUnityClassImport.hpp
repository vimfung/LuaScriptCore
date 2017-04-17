//
//  LuaUnityClassImport.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/1.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityClassImport_hpp
#define LuaUnityClassImport_hpp

#include <stdio.h>
#include <string>
#include <list>
#include "LuaClassImport.h"
#include "LuaUnityDefined.h"

/**
 Unity类型导入
 */
class LuaUnityClassImport : public cn::vimfung::luascriptcore::modules::oo::LuaClassImport
{
private:
    
    /**
     是否允许导出类型处理器
     */
    LuaAllowExportsClassHandlerPtr _allowExportsClassHandlerPtr = NULL;
    
    /**
     获取所有导出类方法处理器
     */
    LuaAllExportClassMethodHandlerPtr _allExportClassMethodHandlerPtr = NULL;
    
    /**
     获取所有导出实例方法处理器
     */
    LuaAllExportInstanceMethodHandlerPtr _allExportInstanceMethodHandlerPtr = NULL;
    
    /**
     获取所有导出实例字段Getter处理器
     */
    LuaAllExportFieldGetterHandlerPtr _allExportInstanceFieldGetterHandlerPtr = NULL;
    
    /**
     获取所有导出实例字段Setter处理器
     */
    LuaAllExportFieldSetterHandlerPtr _allExportInstanceFieldSetterHandlerPtr = NULL;
    
    /**
     创建对象处理器
     */
    LuaCreateNativeObjectHandlerPtr _createObjectHandlerPtr = NULL;
    
    /**
     类方法调用处理器
     */
    LuaNativeClassMethodInvokeHandlerPtr _classMethodInvokeHandlerPtr = NULL;
    
    /**
     实例方法调用处理器
     */
    LuaNativeInstanceMethodInvokeHandlerPtr _instanceMethodInvokeHandlerPtr = NULL;
    
    /**
     字段Getter处理器
     */
    LuaNativeFieldGetterHandlerPtr _fieldGetterHandlerPtr = NULL;
    
    /**
     字段Setter处理器
     */
    LuaNativeFieldSetterHandlerPtr _fieldSetterHandlerPtr = NULL;
    
public:
    
    /**
     初始化
     */
    LuaUnityClassImport();
    
    /**
     注册ClassImport

     @param name 模块名称
     @param context 上下文对象
     */
    void onRegister(const std::string &name, cn::vimfung::luascriptcore::LuaContext *context);
    
public:
    
    /**
     设置是否允许导出类型处理器

     @param handler 处理器
     */
    void setAllowExportsClassHandler(LuaAllowExportsClassHandlerPtr handler);
    
    /**
     设置获取所有导出类方法处理器

     @param handler 处理器
     */
    void setAllExportClassMethodHandler(LuaAllExportClassMethodHandlerPtr handler);
    
    /**
     设置获取所有导出实例方法处理器

     @param handler 处理器
     */
    void setAllExportInstanceMethodHandler(LuaAllExportInstanceMethodHandlerPtr handler);
    
    /**
     设置获取所有导出实例字段Getter处理器

     @param handler 处理器
     */
    void setAllExportInstanceFieldGetterHandler(LuaAllExportFieldGetterHandlerPtr handler);
    
    /**
     设置获取所有导出实例字段Setter处理器

     @param handler 处理器
     */
    void setAllExportInstanceFieldSetterHandler(LuaAllExportFieldSetterHandlerPtr handler);
    
    /**
     创建对象处理器

     @param handler 处理器
     */
    void setCreateObjectHandler(LuaCreateNativeObjectHandlerPtr handler);
    
    /**
     设置类方法调用处理器

     @param handler 处理器
     */
    void setClassMethodInvokeHandler(LuaNativeClassMethodInvokeHandlerPtr handler);
    
    /**
     设置实例方法调用处理器

     @param handler 处理器
     */
    void setInstanceMethodInvokeHandler(LuaNativeInstanceMethodInvokeHandlerPtr handler);
    
    /**
     设置字段Getter处理器

     @param handler 处理器
     */
    void setFieldGetterHandler(LuaNativeFieldGetterHandlerPtr handler);
    
    /**
     设置字段Setter处理器

     @param handler 处理器
     */
    void setFieldSetterHandler(LuaNativeFieldSetterHandlerPtr handler);
    
public:
    
    /**
     获取是否允许导出类型处理器
     
     @return 处理器
     */
    LuaAllowExportsClassHandlerPtr getAllowExportsClassHandler();
    
    /**
     获取获取所有导出类方法处理器
     
     @return 处理器
     */
    LuaAllExportClassMethodHandlerPtr getAllExportClassMethodHandler();
    
    /**
     获取获取所有导出实例方法处理器
     
     @return 处理器
     */
    LuaAllExportInstanceMethodHandlerPtr getAllExportInstanceMethodHandler();
    
    /**
     获取获取所有导出实例字段Getter处理器
     
     @return 处理器
     */
    LuaAllExportFieldGetterHandlerPtr getAllExportInstanceFieldGetterHandler();
    
    /**
     获取获取所有导出实例字段Setter处理器
     
     @return 处理器
     */
    LuaAllExportFieldSetterHandlerPtr getAllExportInstanceFieldSetterHandler();
    
    /**
     获取创建对象处理器
     
     @return 处理器
     */
    LuaCreateNativeObjectHandlerPtr getCreateObjectHandler();
    
    /**
     获取类方法调用处理器
     
     @return 处理器
     */
    LuaNativeClassMethodInvokeHandlerPtr getClassMethodInvokeHandler();
    
    /**
     获取实例方法调用处理器
     
     @return 处理器
     */
    LuaNativeInstanceMethodInvokeHandlerPtr getInstanceMethodInvokeHandler();
    
    /**
     获取字段Getter处理器
     
     @return 处理器
     */
    LuaNativeFieldGetterHandlerPtr getFieldGetterHandler();
    
    /**
     获取字段Setter处理器
     
     @return 处理器
     */
    LuaNativeFieldSetterHandlerPtr getFieldSetterHandler();
};

#endif /* LuaUnityClassImport_hpp */
