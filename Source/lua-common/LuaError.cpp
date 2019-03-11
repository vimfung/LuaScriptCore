//
// Created by 冯鸿杰 on 2019/3/11.
//

#include "LuaError.h"
#include "LuaSession.h"

using namespace cn::vimfung::luascriptcore;

LuaError::LuaError(LuaSession *session, std::string const& message)
{
    _session = session;
    _session -> retain();

    _message = message;
}

LuaError::~LuaError()
{
    _session -> release();
    _session = NULL;
}

std::string LuaError::getMessage()
{
    return _message;
}

LuaSession* LuaError::getSession()
{
    return _session;
}