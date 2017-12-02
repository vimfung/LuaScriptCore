//
//  LuaExportPropertyDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/30.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaExportPropertyDescriptor.hpp"

using namespace cn::vimfung::luascriptcore;

LuaExportPropertyDescriptor::LuaExportPropertyDescriptor(std::string name, bool canRead, bool canWrite)
{
    _name = name;
    _canRead = canRead;
    _canWrite = canWrite;
}

bool LuaExportPropertyDescriptor::canRead()
{
    return _canRead;
}

bool LuaExportPropertyDescriptor::canWrite()
{
    return _canWrite;
}

std::string LuaExportPropertyDescriptor::name()
{
    return _name;
}

LuaValue* LuaExportPropertyDescriptor::invokeGetter(LuaSession *session, LuaObjectDescriptor *instance)
{
    return NULL;
}

void LuaExportPropertyDescriptor::invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value)
{
    
}
