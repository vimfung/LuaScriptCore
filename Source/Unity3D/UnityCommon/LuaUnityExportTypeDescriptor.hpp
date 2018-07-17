//
//  LuaUnityExportTypeDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/27.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityExportTypeDescriptor_hpp
#define LuaUnityExportTypeDescriptor_hpp

#include <stdio.h>
#include "LuaExportTypeDescriptor.hpp"
#include "LuaUnityDefined.h"

using namespace cn::vimfung::luascriptcore;

/**
 Unity导出类型
 */
class LuaUnityExportTypeDescriptor : public LuaExportTypeDescriptor
{
public:
    /**
     初始化对象
     
     @param name 类型名称
     @param parentTypeDescriptor 父级类型
     */
    LuaUnityExportTypeDescriptor(std::string const& name, LuaExportTypeDescriptor *parentTypeDescriptor);
    
public:
    
    /**
     创建对象实例处理器
     */
    LuaInstanceCreateHandlerPtr createInstanceHandler;
    
    /**
     销毁对象实例处理器
     */
    LuaInstanceDestoryHandlerPtr destroyInstanceHandler;
    
    /**
     实例描述处理器
     */
    LuaInstanceDescriptionHandlerPtr instanceDescriptionHandler;
    
public:
    
    /**
     创建实例
     
     @param session 会话
     */
    virtual LuaObjectDescriptor* createInstance(LuaSession *session);
    
    /**
     销毁实例
     
     @param session 会话
     @param objectDescriptor 实例对象
     */
    virtual void destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor);
    
    /**
     创建子类型
     
     @param session 会话
     @param subTypeName 子类型名称
     @return 类型
     */
    virtual LuaExportTypeDescriptor* createSubType(LuaSession *session, std::string const& subTypeName);
    
};

#endif /* LuaUnityExportTypeDescriptor_hpp */
