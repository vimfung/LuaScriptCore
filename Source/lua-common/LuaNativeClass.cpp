//
//  LuaNativeClass.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#include "LuaNativeClass.hpp"
#include "LuaNativeClassFactory.hpp"
#include <map>

using namespace cn::vimfung::luascriptcore;

LuaNativeClass::LuaNativeClass(std::string className,
                               CreateInstanceHandler createInstanceHandler,
                               CreateInstanceByDecoderHandler createInstanceByDecoderHandler)
{
    _className = className;
    _createInstanceHandler = createInstanceHandler;
    _createInstanceByDecoderHandler = createInstanceByDecoderHandler;
    
    LuaNativeClass::registerClass(className, this);
}

void* LuaNativeClass::createInstance()
{
    if (_createInstanceHandler)
    {
        return _createInstanceHandler();
    }
    
    return NULL;
}

void* LuaNativeClass::createInstance(LuaObjectDecoder *decoder)
{
    if (_createInstanceByDecoderHandler)
    {
        return _createInstanceByDecoderHandler(decoder);
    }
    
    return NULL;
}

void LuaNativeClass::registerClass(std::string className, LuaNativeClass* nativeClass)
{
    LuaNativeClassFactory::shareInstance().registerClass(className, nativeClass);
}

LuaNativeClass* LuaNativeClass::findClass(std::string className)
{
    return LuaNativeClassFactory::shareInstance().findClass(className);
}
