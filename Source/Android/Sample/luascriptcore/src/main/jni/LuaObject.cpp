//
// Created by vimfung on 16/8/23.
//

#include "LuaObject.h"

cn::vimfung::luascriptcore::LuaObject::LuaObject()
{
    _retainCount = 1;
}

cn::vimfung::luascriptcore::LuaObject::~LuaObject()
{

}

void cn::vimfung::luascriptcore::LuaObject::retain()
{
    _retainCount ++;
}

void cn::vimfung::luascriptcore::LuaObject::release()
{
    _retainCount --;
    if (_retainCount <= 0)
    {
        delete this;
    }
}