//
//  LuaUnityExportClassProxy.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/5.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityExportClassProxy_hpp
#define LuaUnityExportClassProxy_hpp

#include <stdio.h>
#include <string>
#include "LuaUnityDefined.h"
#include "LuaExportClassProxy.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaObjectDescriptor;
            class LuaContext;
        }
    }
}

/**
 Unity导出类型代理
 */
class LuaUnityExportClassProxy : public cn::vimfung::luascriptcore::modules::oo::LuaExportClassProxy
{
private:
    cn::vimfung::luascriptcore::LuaContext *_context;
    cn::vimfung::luascriptcore::LuaObjectDescriptor *_classDescriptor;
    
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

public:
    
    /**
     初始化
     
     @param className 类名
     @param allExportClassMethodHandlerPtr 导出类方法处理器
     @param allExportInstanceMethodHandlerPtr 导出实例方法处理器
     @param allExportInstanceFieldGetterHandlerPtr 导出字段Getter处理器
     @param allExportInstanceFieldSetterHandlerPtr 导出字段Setter处理器
     */
    LuaUnityExportClassProxy(cn::vimfung::luascriptcore::LuaContext *context,
                             std::string className,
                             LuaAllExportClassMethodHandlerPtr allExportClassMethodHandlerPtr,
                             LuaAllExportInstanceMethodHandlerPtr allExportInstanceMethodHandlerPtr,
                             LuaAllExportFieldGetterHandlerPtr allExportInstanceFieldGetterHandlerPtr,
                             LuaAllExportFieldSetterHandlerPtr allExportInstanceFieldSetterHandlerPtr);
    
    
    /**
     释放对象
     */
    virtual ~LuaUnityExportClassProxy();
    
    /**
     * 获取类型
     *
     * @return 类型对象描述器
     */
    cn::vimfung::luascriptcore::LuaObjectDescriptor* getExportClass();
    
    /**
     * 获取所有类方法
     *
     * @return 方法名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportClassMethods();
    
    /**
     * 获取所有实例方法
     *
     * @return 方法名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportInstanceMethods();
    
    /**
     * 获取所有读权限字段
     *
     * @return 字段名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportGetterFields();
    
    /**
     * 获取所有写权限字段
     *
     * @return 字段名称列表
     */
    cn::vimfung::luascriptcore::modules::oo::LuaExportNameList allExportSetterFields();
};

#endif /* LuaUnityExportClassProxy_hpp */
