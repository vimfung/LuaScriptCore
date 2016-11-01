//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaObjectDescriptor.h"
#include "LuaContext.h"

/**
 对象引用回收处理

 @param state Lua状态机

 @return 返回值数量
 */
static int objectReferenceGCHandler(lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;

    //释放对象
    LuaObjectDescriptor **ref = (LuaObjectDescriptor **)lua_touserdata(state, 1);
    (*ref) -> release();

    return 0;
}

cn::vimfung::luascriptcore::LuaObjectDescriptor::LuaObjectDescriptor()
        : _object(NULL)
{

}

cn::vimfung::luascriptcore::LuaObjectDescriptor::LuaObjectDescriptor(const void *object)
{
    setObject(object);
}

cn::vimfung::luascriptcore::LuaObjectDescriptor::~LuaObjectDescriptor()
{
    _object = NULL;
}

void cn::vimfung::luascriptcore::LuaObjectDescriptor::setObject(const void *object)
{
    _object = (void *)object;
}

void cn::vimfung::luascriptcore::LuaObjectDescriptor::setUserdata(void *userdata)
{
    _userData = userdata;
}

const void* cn::vimfung::luascriptcore::LuaObjectDescriptor::getUserdata()
{
    return _userData;
}

const void* cn::vimfung::luascriptcore::LuaObjectDescriptor::getObject()
{
    return _object;
}

void cn::vimfung::luascriptcore::LuaObjectDescriptor::push(cn::vimfung::luascriptcore::LuaContext *context)
{
    lua_State *state = context -> getLuaState();

    LuaObjectDescriptor **ref = (LuaObjectDescriptor **)lua_newuserdata(state, sizeof(LuaObjectDescriptor **));
    //创建本地实例对象，赋予lua的内存块
    *ref = this;
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