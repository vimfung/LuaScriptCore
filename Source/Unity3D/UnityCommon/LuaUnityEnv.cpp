//
//  LuaUnityEnv.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/23.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityEnv.hpp"

LuaUnityEnv* LuaUnityEnv::sharedInstance()
{
    static LuaUnityEnv *env = NULL;
    
    if (env == NULL)
    {
        env = new LuaUnityEnv();
    }
    
    return env;
}

LuaUnityEnv::LuaUnityEnv()
{
    _setNativeObjectIdHandler = NULL;
}

void LuaUnityEnv::bindSetNativeObjectIdHandler (LuaSetNativeObjectIdHandlerPtr handler)
{
    _setNativeObjectIdHandler = handler;
}

void LuaUnityEnv::bindGetClassNameByInstanceHandler (LuaGetClassNameByInstanceHandlerPtr handler)
{
    _getClassNameByInstanceHandler = handler;
}

void LuaUnityEnv::bindExportsNativeTypeHandler (LuaExportsNativeTypeHandlerPtr handler)
{
    _exportsNativeTypeHandler = handler;
}

void LuaUnityEnv::setNativeObjectId(const void *instance, int nativeObjectId, std::string const& luaObjectId)
{
    if (_setNativeObjectIdHandler != NULL)
    {
        _setNativeObjectIdHandler ((long long)instance, nativeObjectId, luaObjectId.c_str());
    }
}

std::string LuaUnityEnv::getClassNameByInstance(const void *instance)
{
    std::string clsName;
    
    if (_getClassNameByInstanceHandler != NULL)
    {
        clsName = _getClassNameByInstanceHandler(instance);
    }
    
    return clsName;
}

void LuaUnityEnv::exportsNativeType(int contextId, std::string const& typeName)
{
    if (_exportsNativeTypeHandler != NULL)
    {
        _exportsNativeTypeHandler(contextId, typeName.c_str());
    }
}
