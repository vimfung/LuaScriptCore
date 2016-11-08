//
// Created by vimfung on 16/8/23.
//

#include <stddef.h>
#include "LuaValue.h"
#include "LuaDefine.h"
#include "LuaContext.h"
#include "../../../../../lua-core/src/lua.h"
#include "LuaObjectManager.h"
#include "LuaPointer.h"

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
    ((LuaObjectDescriptor *)ref -> value) -> destroyReference();

    return 0;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue()
        : LuaObject()
{
    _type = LuaValueTypeNil;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(long value)
        : LuaObject()
{
    _type = LuaValueTypeInteger;
    _intValue = (lua_Integer)value;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(bool value)
        : LuaObject()
{
    _type = LuaValueTypeBoolean;
    _booleanValue = value;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(double value)
        : LuaObject()
{
    _type = LuaValueTypeNumber;
    _numberValue = value;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(std::string value)
        : LuaObject()
{
    _type = LuaValueTypeString;
    _value = new std::string(value);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(const char *bytes, size_t length)
        : LuaObject()
{
    _type = LuaValueTypeData;
    _bytesLen = length;
    _value = new char[_bytesLen];
    memcpy(_value, bytes, _bytesLen);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(LuaValueList value)
        : LuaObject()
{
    _type = LuaValueTypeArray;
    _value = new LuaValueList(value);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(LuaValueMap value)
        : LuaObject()
{
    _type = LuaValueTypeMap;
    _value = new LuaValueMap (value);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue (LuaPointer *value)
        :LuaObject()
{
    _type = LuaValueTypePtr;

    value -> retain();
    _value = (void *)value;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue (LuaObjectDescriptor *value)
{
    _type = LuaValueTypeObject;

    value -> retain();
    _value = (void *)value;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(LuaFunction *value)
{
    _type = LuaValueTypeFunction;

    value -> retain();
    _value = (void *)value;
}


cn::vimfung::luascriptcore::LuaValue::~LuaValue()
{
    if (_value != NULL)
    {
        if (_type == LuaValueTypeArray)
        {
            //对于Table类型需要释放其子对象内存
            LuaValueList *arrayValue = static_cast<LuaValueList *> (_value);
            if (arrayValue != NULL)
            {
                //为数组对象
                for (LuaValueList::iterator i = arrayValue -> begin(); i != arrayValue -> end(); ++i)
                {
                    LuaValue *value = *i;
                    value -> release();
                }
            }
        }
        else if (_type == LuaValueTypeMap)
        {
            //为字典对象
            LuaValueMap *mapValue = static_cast<LuaValueMap *> (_value);
            if (mapValue != NULL)
            {
                for (LuaValueMap::iterator i = mapValue -> begin(); i != mapValue -> end(); ++i)
                {
                    i->second->release();
                }
            }
        }
        else if (_type == LuaValueTypePtr || _type == LuaValueTypeObject || _type == LuaValueTypeFunction)
        {
            ((LuaObject *)_value) -> release();
        }

        if (_type != LuaValueTypePtr && _type != LuaValueTypeObject && _type != LuaValueTypeFunction)
        {
            delete _value;
        }

        _value = NULL;
    }
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::NilValue()
{
    return new LuaValue();
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::IntegerValue(long value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::BooleanValue(bool value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::NumberValue(double value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::StringValue(std::string value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::DataValue(const char *bytes, size_t length)
{
    return new LuaValue(bytes, length);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::ArrayValue(LuaValueList value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::DictonaryValue(LuaValueMap value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::PointerValue(LuaPointer *value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::FunctionValue(LuaFunction *value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::ObjectValue(LuaObjectDescriptor *value)
{
    return new LuaValue(value);
}

cn::vimfung::luascriptcore::LuaValueType cn::vimfung::luascriptcore::LuaValue::getType()
{
    return _type;
}

void cn::vimfung::luascriptcore::LuaValue::push(cn::vimfung::luascriptcore::LuaContext *context)
{
    pushValue(context, this);
}

void cn::vimfung::luascriptcore::LuaValue::pushValue(cn::vimfung::luascriptcore::LuaContext *context, cn::vimfung::luascriptcore::LuaValue *value)
{
    lua_State *state = context -> getLuaState();

    switch (value -> getType())
    {
        case LuaValueTypeInteger:
            lua_pushinteger(state, value -> _intValue);
            break;
        case LuaValueTypeNumber:
            lua_pushnumber(state, value -> _numberValue);
            break;
        case LuaValueTypeNil:
            lua_pushnil(state);
            break;
        case LuaValueTypeString:
            lua_pushstring(state, ((std::string *)value -> _value) -> c_str());
            break;
        case LuaValueTypeBoolean:
            lua_pushboolean(state, value -> _booleanValue);
            break;
        case LuaValueTypeArray:
            pushTable(context, static_cast<LuaValueList *> (value -> _value));
            break;
        case LuaValueTypeMap:
            pushTable(context, static_cast<LuaValueMap *> (value -> _value));
            break;
        case LuaValueTypeData:
            lua_pushlstring(state, (char *)value -> _value, value -> _bytesLen);
            break;
        case LuaValueTypePtr:
            lua_pushlightuserdata(state, (void *)value -> toPointer() -> getValue());
            break;
        case LuaValueTypeFunction:
            value -> toFunction() -> push();
            break;
        case LuaValueTypeObject:
        {
            value -> toObject() -> push(context);
            break;
        }
        default:
            break;
    }
}

void cn::vimfung::luascriptcore::LuaValue::pushTable(cn::vimfung::luascriptcore::LuaContext *context, LuaValueList *list)
{
    lua_State *state = context -> getLuaState();

    lua_newtable(state);

    lua_Integer index = 1;
    for (LuaValueList::iterator it = list -> begin(); it != list -> end(); ++it)
    {
        LuaValue *item = *it;
        pushValue(context, item);
        lua_rawseti(state, -2, index);

        index ++;
    }
}

void cn::vimfung::luascriptcore::LuaValue::pushTable(cn::vimfung::luascriptcore::LuaContext *context, LuaValueMap *map)
{
    lua_State *state = context -> getLuaState();

    lua_newtable(state);

    for (LuaValueMap::iterator it = map -> begin(); it != map -> end() ; ++it)
    {
        LuaValue *item = it -> second;
        pushValue(context, item);
        lua_setfield(state, -2, it -> first.c_str());
    }
}

long cn::vimfung::luascriptcore::LuaValue::toInteger()
{
    if (_type == LuaValueTypeInteger)
    {
        return _intValue;
    }
    return 0;
}


const std::string cn::vimfung::luascriptcore::LuaValue::toString()
{
    if (_type == LuaValueTypeString)
    {
        return *((const std::string *)_value);
    }

    return NULL;
}

double cn::vimfung::luascriptcore::LuaValue::toNumber()
{
    if (_type == LuaValueTypeNumber)
    {
        return _numberValue;
    }

    return 0;
}

bool cn::vimfung::luascriptcore::LuaValue::toBoolean()
{
    if (_type == LuaValueTypeBoolean)
    {
        return  _booleanValue;
    }

    return false;
}

const char* cn::vimfung::luascriptcore::LuaValue::toData()
{
    if (_type == LuaValueTypeData)
    {
        return (const char *)_value;
    }

    return NULL;
}

size_t cn::vimfung::luascriptcore::LuaValue::getDataLength()
{
    if (_type == LuaValueTypeData)
    {
        return _bytesLen;
    }

    return 0;
}

cn::vimfung::luascriptcore::LuaValueList* cn::vimfung::luascriptcore::LuaValue::toArray()
{
    if (_type == LuaValueTypeArray)
    {
        return static_cast<LuaValueList *>(_value);
    }

    return NULL;
}

cn::vimfung::luascriptcore::LuaValueMap* cn::vimfung::luascriptcore::LuaValue::toMap()
{
    if (_type == LuaValueTypeMap)
    {
        return static_cast<LuaValueMap *>(_value);
    }

    return NULL;
}

cn::vimfung::luascriptcore::LuaPointer* cn::vimfung::luascriptcore::LuaValue::toPointer()
{
    if (_type == LuaValueTypePtr)
    {
        return (LuaPointer *)_value;
    }

    return NULL;
}

cn::vimfung::luascriptcore::LuaFunction* cn::vimfung::luascriptcore::LuaValue::toFunction()
{
    if (_type == LuaValueTypeFunction)
    {
        return (LuaFunction *)_value;
    }

    return NULL;
}

cn::vimfung::luascriptcore::LuaObjectDescriptor* cn::vimfung::luascriptcore::LuaValue::toObject()
{
    if (_type == LuaValueTypeObject)
    {
        return (LuaObjectDescriptor *)_value;
    }
    else if (_type == LuaValueTypePtr)
    {
        return (LuaObjectDescriptor *)((LuaPointer *)_value) -> getValue() -> value;
    }

    return NULL;
}