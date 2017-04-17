//
//  LuaUnityClassObjectDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/7.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityClassObjectDescriptor_hpp
#define LuaUnityClassObjectDescriptor_hpp

#include <stdio.h>
#include <string>
#include "LuaObjectDescriptor.h"

/**
 类型对象描述
 */
class LuaUnityClassObjectDescriptor : public cn::vimfung::luascriptcore::LuaObjectDescriptor
{
private:
    std::string _className;
public:
    LuaUnityClassObjectDescriptor(std::string className);
};

#endif /* LuaUnityClassObjectDescriptor_hpp */
