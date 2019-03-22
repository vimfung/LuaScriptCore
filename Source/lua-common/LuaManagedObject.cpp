//
// Created by 冯鸿杰 on 2017/5/12.
//

#include "LuaManagedObject.h"
#include "LuaObjectDecoder.hpp"
#include "LuaContext.h"
#include "LuaDataExchanger.h"

using namespace cn::vimfung::luascriptcore;

LuaManagedObject::LuaManagedObject(LuaContext *context)
    : LuaObject(), _context(context)
{

}

LuaManagedObject::LuaManagedObject (LuaObjectDecoder *decoder)
    : LuaObject (decoder), _context(decoder -> getContext())
{

}

void LuaManagedObject::push(LuaContext *context)
{

}

void LuaManagedObject::push(lua_State *state, LuaOperationQueue *queue)
{

}

void LuaManagedObject::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
}

LuaManagedObject::~LuaManagedObject()
{
    //清除对象在交互层的引用
    if (_context != NULL)
    {
        _context -> getDataExchanger() -> clearObject(this);
    }
}

LuaContext *LuaManagedObject::getContext()
{
    return _context;
}

std::string LuaManagedObject::getExchangeId()
{
    return _exchangeId;
}
