//
// Created by 冯鸿杰 on 2019/3/12.
//

#include "LuaScriptController.h"

using namespace cn::vimfung::luascriptcore;

LuaScriptController::LuaScriptController()
{
    timeout = 0;
    startTime = 0;
    isForceExit = false;
}

void LuaScriptController::setTimeout(int timeout)
{
    this -> timeout = timeout;
}

void LuaScriptController::forceExit()
{
    this -> isForceExit = true;
}