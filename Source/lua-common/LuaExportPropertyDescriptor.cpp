//
//  LuaExportPropertyDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/30.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaExportPropertyDescriptor.hpp"
#include "LuaFunction.h"
#include "LuaValue.h"

using namespace cn::vimfung::luascriptcore;

LuaExportPropertyDescriptor::LuaExportPropertyDescriptor(std::string name, bool canRead, bool canWrite)
{
    _name = name;
    _canRead = canRead;
    _canWrite = canWrite;
    _getter = NULL;
    _setter = NULL;
}

LuaExportPropertyDescriptor::LuaExportPropertyDescriptor(std::string name, LuaFunction *getter, LuaFunction *setter)
{
    _name = name;
    
    if (getter != NULL)
    {
        getter -> retain();
        _getter = getter;
    }
    
    if (setter != NULL)
    {
        setter -> retain();
        _setter = setter;
    }
    
    _canRead = getter != NULL;
    _canWrite = setter != NULL;
}

LuaExportPropertyDescriptor::~LuaExportPropertyDescriptor()
{
    if (_getter != NULL)
    {
        _getter -> release();
        _getter = NULL;
    }
    
    if (_setter != NULL)
    {
        _setter -> release();
        _setter = NULL;
    }
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
    LuaValue *retValue = NULL;
    if (_getter != NULL)
    {
        //调用
        LuaArgumentList args;
        args.push_back(LuaValue::ObjectValue(instance));
        retValue = _getter -> invoke(&args);
    }
    
    return retValue;
}

void LuaExportPropertyDescriptor::invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value)
{
    if (_setter != NULL)
    {
        LuaArgumentList args;
        args.push_back(LuaValue::ObjectValue(instance));
        args.push_back(value);
        _setter -> invoke(&args);
    }
}
