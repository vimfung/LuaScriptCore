//
// Created by vimfung on 16/8/23.
//

#include "LuaObject.h"
#include "LuaDefine.h"

static int _objSeqId = 0;

cn::vimfung::luascriptcore::LuaObject::LuaObject()
{
    _retainCount = 1;

    _objSeqId ++;
    _objectId = _objSeqId;
}

cn::vimfung::luascriptcore::LuaObject::~LuaObject()
{

}

int cn::vimfung::luascriptcore::LuaObject::objectId()
{
    return _objectId;
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