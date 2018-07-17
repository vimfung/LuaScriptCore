//
//  LuaUnityExportPropertyDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/30.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityExportPropertyDescriptor_hpp
#define LuaUnityExportPropertyDescriptor_hpp

#include <stdio.h>
#include "LuaExportPropertyDescriptor.hpp"
#include "LuaUnityDefined.h"

using namespace cn::vimfung::luascriptcore;

/**
 Unity导出属性描述
 */
class LuaUnityExportPropertyDescriptor : public LuaExportPropertyDescriptor
{
public:
    
    /**
     初始化
     
     @param name 属性名称
     @param canRead 是否可读
     @param canWrite 是否可写
     @param getterHandler getter处理器
     @param setterHandler  setter处理器
     */
    LuaUnityExportPropertyDescriptor(std::string const& name,
                                     bool canRead,
                                     bool canWrite,
                                     LuaInstanceFieldGetterHandlerPtr getterHandler,
                                     LuaInstanceFieldSetterHandlerPtr setterHandler);
    
    /**
     调用Getter方法
     
     @param session 会话
     @param instance 实例对象
     @return 返回值
     */
    virtual LuaValue* invokeGetter(LuaSession *session, LuaObjectDescriptor *instance);
    
    
    /**
     调用Setter方法
     
     @param session 会话
     @param instance 实例对象
     @param value 属性值
     */
    virtual void invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value);
    
private:
    
    /**
     Getter处理器
     */
    LuaInstanceFieldGetterHandlerPtr _getterHandler;
    
    /**
     Setter处理器
     */
    LuaInstanceFieldSetterHandlerPtr _setterHandler;
};

#endif /* LuaUnityExportPropertyDescriptor_hpp */
