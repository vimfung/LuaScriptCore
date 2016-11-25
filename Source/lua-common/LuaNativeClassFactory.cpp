//
//  LuaNativeClassFactory.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#include "LuaNativeClassFactory.hpp"

using namespace cn::vimfung::luascriptcore;

LuaNativeClassFactory& LuaNativeClassFactory::shareInstance()
{
    static LuaNativeClassFactory factory;
    return factory;
}

void LuaNativeClassFactory::registerClass(std::string className, LuaNativeClass* nativeClass)
{
    _classMap[className] = nativeClass;
}

LuaNativeClass* LuaNativeClassFactory::findClass(std::string className)
{
    LuaClassMap::iterator it = _classMap.find(className);
    if (it != _classMap.end())
    {
        return it -> second;
    }
    
    return NULL;
}
