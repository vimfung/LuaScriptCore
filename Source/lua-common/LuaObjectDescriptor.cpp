//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaObjectDescriptor.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaContext.h"
#include "LuaNativeClass.hpp"
#include "LuaEngineAdapter.hpp"
#include "StringUtils.h"
#include "LuaSession.h"
#include "LuaExportTypeDescriptor.hpp"
#include "LuaExportsTypeManager.hpp"
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
    LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::toUserdata(state, 1);
    LuaObjectDescriptor *descriptor = (LuaObjectDescriptor *)(ref -> value);
    descriptor -> release();

    return 0;
}

LuaObjectDescriptor::LuaObjectDescriptor()
        : _object(NULL), _typeDescriptor(NULL)
{
    _linkId = StringUtils::format("%p", this);
}

LuaObjectDescriptor::LuaObjectDescriptor(const void *object)
    : _object((void *)object), _typeDescriptor(NULL)
{
    _linkId = StringUtils::format("%p", this);
}

LuaObjectDescriptor::LuaObjectDescriptor(void *object, LuaExportTypeDescriptor *typeDescriptor)
    : _object(object), _typeDescriptor(typeDescriptor)
{
    _linkId = StringUtils::format("%p", this);
}

LuaObjectDescriptor::LuaObjectDescriptor (LuaObjectDecoder *decoder)
    : LuaManagedObject(decoder)
{
    void *objRef = NULL;
    objRef = (void *)decoder -> readInt64();
    setObject(objRef);

    _linkId = decoder -> readString();
    
    //读取类型
    int typeId = decoder -> readInt32();
    if (typeId > 0)
    {
        _typeDescriptor = (LuaExportTypeDescriptor *)LuaObject::findObject(typeId);
    }
    
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

LuaExportTypeDescriptor* LuaObjectDescriptor::getTypeDescriptor()
{
    return _typeDescriptor;
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
    
    if (_typeDescriptor != NULL)
    {
        //如果为导出类型
        context -> getExportsTypeManager() -> createLuaObject(this);
        return;
    }

    lua_State *state = context -> getCurrentSession() -> getState();

    //创建userdata
    LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::newUserdata(state, sizeof(LuaUserdataRef));
    ref -> value = this;
    this -> retain();

    //设置userdata的元表
    LuaEngineAdapter::getMetatable(state, "_ObjectReference_");
    if (LuaEngineAdapter::isNil(state, -1))
    {
        LuaEngineAdapter::pop(state, 1);

        //尚未注册_ObjectReference,开始注册对象
        LuaEngineAdapter::newMetatable(state, "_ObjectReference_");

        LuaEngineAdapter::pushCFunction(state, objectReferenceGCHandler);
        LuaEngineAdapter::setField(state, -2, "__gc");
    }
    
    LuaEngineAdapter::setMetatable(state, -2);
}

void LuaObjectDescriptor::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    
    encoder -> writeInt64((long long)_object);
    encoder -> writeString(_linkId);
    
    //写入类型标识
    if (_typeDescriptor != NULL)
    {
        encoder -> writeInt32(_typeDescriptor -> objectId());
    }
    else
    {
        encoder -> writeInt32(0);
    }
    
    //写入自定义数据
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
