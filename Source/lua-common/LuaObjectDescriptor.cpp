//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaObjectDescriptor.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaContext.h"
#include "LuaNativeClass.hpp"
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaObjectDescriptor);

/**
 对象引用回收处理

 @param state Lua状态机

 @return 返回值数量
 */
static int objectReferenceGCHandler(lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;

    //释放对象
    LuaUserdataRef ref = (LuaUserdataRef)lua_touserdata(state, 1);
    LuaObjectDescriptor *descriptor = (LuaObjectDescriptor *)(ref -> value);
    descriptor -> release();

    return 0;
}

LuaObjectDescriptor::LuaObjectDescriptor()
        : _object(NULL)
{

}

LuaObjectDescriptor::LuaObjectDescriptor(const void *object)
{
    setObject(object);
}

LuaObjectDescriptor::LuaObjectDescriptor (LuaObjectDecoder *decoder)
    : LuaObject(decoder)
{
    void *objRef = NULL;
    objRef = (void *)decoder -> readInt64();
    setObject(objRef);
    
    setReferenceId(decoder -> readString());
}

LuaObjectDescriptor::~LuaObjectDescriptor()
{
    _object = NULL;
}

std::string LuaObjectDescriptor::typeName()
{
    static std::string name = typeid(LuaObjectDescriptor).name();
    return name;
}

void LuaObjectDescriptor::setObject(const void *object)
{
    _object = (void *)object;
}

const void* LuaObjectDescriptor::getObject()
{
    return _object;
}

void LuaObjectDescriptor::setReferenceId(const std::string &refId)
{
    _refId = refId;
}

std::string LuaObjectDescriptor::getReferenceId()
{
    return _refId;
}

void LuaObjectDescriptor::push(LuaContext *context)
{
    lua_State *state = context -> getLuaState();

    //创建userdata
    LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
    ref -> value = this;
    this -> retain();

    //设置userdata的元表
    luaL_getmetatable(state, "_ObjectReference_");
    if (lua_isnil(state, -1))
    {
        lua_pop(state, 1);

        //尚未注册_ObjectReference,开始注册对象
        luaL_newmetatable(state, "_ObjectReference_");

        lua_pushcfunction(state, objectReferenceGCHandler);
        lua_setfield(state, -2, "__gc");
    }
    lua_setmetatable(state, -2);
}

void LuaObjectDescriptor::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    
    encoder -> writeInt64((long long)_object);
    encoder -> writeString(_refId);
}
