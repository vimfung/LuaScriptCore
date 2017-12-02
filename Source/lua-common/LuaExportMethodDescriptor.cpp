//
//  LuaExportMethodDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/16.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaExportMethodDescriptor.hpp"
#include "LuaValue.h"

using namespace cn::vimfung::luascriptcore;

LuaExportMethodDescriptor::LuaExportMethodDescriptor(std::string name, std::string methodSignature)
{
    _name = name;
    _methodSignature = methodSignature;
}

std::string LuaExportMethodDescriptor::name()
{
    return _name;
}

std::string LuaExportMethodDescriptor::methodSignature()
{
    return _methodSignature;
}

LuaValue* LuaExportMethodDescriptor::invoke(LuaSession *session, LuaArgumentList arguments)
{
    return NULL;
}
