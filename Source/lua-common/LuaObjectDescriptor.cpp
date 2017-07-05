//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaObjectDescriptor.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaContext.h"
#include "LuaNativeClass.hpp"
#include "StringUtils.h"
#include "LuaSession.h"
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaObjectDescriptor);

/**
 * 过滤列表
 */
static std::list<LuaObjectDescriptorPushFilter> _pushFilters;

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
    _linkId = StringUtils::format("%p", this);
}

LuaObjectDescriptor::LuaObjectDescriptor(const void *object)
{
    _linkId = StringUtils::format("%p", this);

    setObject(object);
}

LuaObjectDescriptor::LuaObjectDescriptor (LuaObjectDecoder *decoder)
    : LuaManagedObject(decoder)
{
    void *objRef = NULL;
    objRef = (void *)decoder -> readInt64();
    setObject(objRef);

    _linkId = decoder -> readString();
    
    //读取用户数据
    int size = decoder -> readInt32();
    for (int i = 0; i < size; i++)
    {
        std::string key = decoder -> readString();
        std::string value = decoder -> readString();
        
        _userdata[key] = value;
    }
}

LuaObjectDescriptor::~LuaObjectDescriptor()
{
    _object = NULL;
}

void LuaObjectDescriptor::addPushFilter(LuaObjectDescriptorPushFilter filter)
{
    _pushFilters.push_back(filter);
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

void LuaObjectDescriptor::setUserdata(std::string key, std::string value)
{
    _userdata[key] = value;
}

std::string LuaObjectDescriptor::getUserdata(std::string key)
{
    std::string retValue;
    
    LuaObjectDescriptorUserData::iterator it = _userdata.find(key);
    if (it != _userdata.end())
    {
        retValue = _userdata[key];
    }
    
    return retValue;
}

void LuaObjectDescriptor::push(LuaContext *context)
{
    //先判断是否有过滤器进行过滤
    for (std::list<LuaObjectDescriptorPushFilter>::iterator it = _pushFilters.begin(); it != _pushFilters.end() ; ++it)
    {
        LuaObjectDescriptorPushFilter filter = *it;
        if (filter != NULL && filter(context, this))
        {
            //返回，不往下执行
            return;
        }
    }

    lua_State *state = context -> getCurrentSession() -> getState();

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
    encoder -> writeString(_linkId);
    
    encoder -> writeInt32((int)_userdata.size());
    for (LuaObjectDescriptorUserData::iterator it = _userdata.begin(); it != _userdata.end(); ++it)
    {
        encoder -> writeString(it -> first);
        encoder -> writeString(it -> second);
    }
}

std::string LuaObjectDescriptor::getLinkId()
{
    return _linkId;
}
