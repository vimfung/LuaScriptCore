//
//  LuaUnityModule.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityModule_hpp
#define LuaUnityModule_hpp

#include <stdio.h>
#include "LuaModule.h"
#include "LuaUnityDefined.h"

using namespace cn::vimfung::luascriptcore;

/**
 Unity模块
 */
class LuaUnityModule : public LuaModule
{
private:
    
    LuaModuleMethodHandlerPtr _methodHandler;
    
public:
    
    /**
     设置类方法处理器
     
     @param handler 处理器
     */
    void setMethodHandler(LuaModuleMethodHandlerPtr handler);
    
    /**
     获取类方法处理器
     
     @return 处理器
     */
    LuaModuleMethodHandlerPtr getMethodHandler();
};

#endif /* LuaUnityModule_hpp */
