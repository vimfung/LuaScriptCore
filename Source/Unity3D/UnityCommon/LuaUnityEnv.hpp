//
//  LuaUnityEnv.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/23.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityEnv_hpp
#define LuaUnityEnv_hpp

#include <stdio.h>
#include "LuaUnityDefined.h"

/**
 Unity环境
 */
class LuaUnityEnv
{
private:
    LuaSetNativeObjectIdHandlerPtr _setNativeObjectIdHandler;
    LuaGetClassNameByInstanceHandlerPtr _getClassNameByInstanceHandler;
    LuaExportsNativeTypeHandlerPtr _exportsNativeTypeHandler;
    LuaUnityEnv();
    
public:
    
    /**
     绑定设置原生对象标识处理器

     @param handler 处理器
     */
    void bindSetNativeObjectIdHandler (LuaSetNativeObjectIdHandlerPtr handler);
    
    /**
     绑定根据实例获取类型名称处理器

     @param handler 处理器
     */
    void bindGetClassNameByInstanceHandler (LuaGetClassNameByInstanceHandlerPtr handler);
    
    /**
     绑定导出原生类型处理器

     @param handler 处理器
     */
    void bindExportsNativeTypeHandler (LuaExportsNativeTypeHandlerPtr handler);
    
public:
    
    /**
     获取共享实例对象
     
     @return Unity环境
     */
    static LuaUnityEnv* sharedInstance();
    
    /**
     设置原生对象ID

     @param instance 实例对象
     @param nativeObjectId 原生对象标识
     @param luaObjectId Lua对象标识
     */
    void setNativeObjectId(const void *instance, int nativeObjectId, std::string const& luaObjectId);
    
    /**
     获取实例的类型名称

     @param instance 实例
     @return 类型名称
     */
    std::string getClassNameByInstance(const void *instance);
    
    /**
     导出原生类型

     @param contextId 上下文标识
     @param typeName 类型名称
     */
    void exportsNativeType(int contextId, std::string const& typeName);
};

#endif /* LuaUnityEnv_hpp */
