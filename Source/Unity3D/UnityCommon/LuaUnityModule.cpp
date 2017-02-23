//
//  LuaUnityModule.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityModule.hpp"

void LuaUnityModule::setMethodHandler(LuaModuleMethodHandlerPtr handler)
{
    _methodHandler = handler;
}

LuaModuleMethodHandlerPtr LuaUnityModule::getMethodHandler()
{
    return _methodHandler;
}
