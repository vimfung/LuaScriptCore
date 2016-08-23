//
// Created by vimfung on 16/8/23.
//

#include <stddef.h>
#include "LuaValue.h"

cn::vimfung::luascriptcore::LuaValue::LuaValue()
{
    _type = LuaValueTypeNil;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(bool value)
{
    _type = LuaValueTypeBoolean;
    _booleanValue = value;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(double value)
{
    _type = LuaValueTypeNumber;
    _numberValue = value;
    _value = NULL;
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(std::string value)
{
    _type = LuaValueTypeString;
    _value = new std::string(value);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(const char *bytes, size_t length)
{
    _type = LuaValueTypeData;
    _bytesLen = length;
    _value = new char[_bytesLen];
    memcpy(_value, bytes, _bytesLen);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(LuaValueList value)
{
    _type = LuaValueTypeTable;
    _value = new LuaValueList(value);
}

cn::vimfung::luascriptcore::LuaValue::LuaValue(LuaValueMap value)
{
    _type = LuaValueTypeTable;
    _value = new LuaValueMap (value);
}


cn::vimfung::luascriptcore::LuaValue::~LuaValue()
{
    if (_value != NULL)
    {
        if (_type == LuaValueTypeTable)
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
            else
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
        }

        delete _value;
        _value = NULL;
    }
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaValue::NilValue()
{
    return new LuaValue();
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

cn::vimfung::luascriptcore::LuaValueType cn::vimfung::luascriptcore::LuaValue::getType()
{
    return _type;
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
    if (_type == LuaValueTypeTable)
    {
        return static_cast<LuaValueList *>(_value);
    }

    return NULL;
}

cn::vimfung::luascriptcore::LuaValueMap* cn::vimfung::luascriptcore::LuaValue::toMap()
{
    if (_type == LuaValueTypeTable)
    {
        return static_cast<LuaValueMap *>(_value);
    }

    return NULL;
}