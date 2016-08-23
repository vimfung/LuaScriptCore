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

cn::vimfung::luascriptcore::LuaValue::~LuaValue()
{
    if (_value != NULL)
    {
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

cn::vimfung::luascriptcore::LuaValueType cn::vimfung::luascriptcore::LuaValue::getType()
{
    return _type;
}


const std::string cn::vimfung::luascriptcore::LuaValue::toString()
{
    switch (_type)
    {
        case LuaValueTypeNumber:
        {
            char buffer[20];
            sprintf(buffer, "%f", _numberValue);
            const std::string str = buffer;
            return str;
        }
        case LuaValueTypeBoolean:
        {
            char buffer[20];
            sprintf(buffer, "%d", _booleanValue);
            const std::string str = buffer;
            return str;
        }
        case LuaValueTypeInteger:
        {
            char buffer[20];
            sprintf(buffer, "%d", _integerValue);
            const std::string str = buffer;
            return str;
        }
        case LuaValueTypeString:
        {
            return *((const std::string *)_value);
        }
        case LuaValueTypeData:
        {
            //转换为16进制字符串描述
            char buffer[10];
            const char *bytes = (const char *)_value;
            std::string str;
            for (int i = 0; i < _bytesLen; ++i)
            {
                sprintf(buffer, "%02x", bytes[i]);
                str.append(buffer);
            }
            return str;
        }
        default:
            return std::string();
    }
}

double cn::vimfung::luascriptcore::LuaValue::toNumber()
{
    switch (_type)
    {
        case LuaValueTypeNumber:
            return _numberValue;
        case LuaValueTypeBoolean:
            return _booleanValue;
        case LuaValueTypeInteger:
            return _integerValue;
        case LuaValueTypeString:
        {
            const char *charArr = (const char *) _value;
            return atof(charArr);
        }
        default:
            return 0;
    }
}