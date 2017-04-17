//
//  LuaUnityClassObjectDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/7.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityClassObjectDescriptor.hpp"

LuaUnityClassObjectDescriptor::LuaUnityClassObjectDescriptor(std::string className)
    :_className(className)
{
    setObject(_className.c_str());
}
