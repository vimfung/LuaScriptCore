//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAVALUE_H
#define SAMPLE_LUAVALUE_H

#include <string>
#include <list>
#include <map>
#include "LuaObject.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaValue;

            typedef std::list<LuaValue *> LuaValueList;
            typedef std::map<std::string, LuaValue*> LuaValueMap;

            enum LuaValueType
            {
                LuaValueTypeNil = 0,
                LuaValueTypeNumber = 1,
                LuaValueTypeBoolean = 2,
                LuaValueTypeString = 3,
                LuaValueTypeArray = 4,
                LuaValueTypeMap = 5,
                LuaValueTypeFunction = 6,
                LuaValueTypeThread = 7,
                LuaValueTypeUserData = 8,
                LuaValueTypeInteger = 9,
                LuaValueTypeData = 10
            };

            class LuaValue : public LuaObject
            {
            private:
                LuaValueType _type;
                bool _booleanValue;
                double _numberValue;
                int _integerValue;
                size_t _bytesLen;
                void *_value;

            public:
                LuaValue ();
                LuaValue (bool value);
                LuaValue (double value);
                LuaValue (std::string value);
                LuaValue (const char *bytes, size_t length);
                LuaValue (LuaValueList value);
                LuaValue (LuaValueMap value);
                ~LuaValue();

            public:
                LuaValueType getType();
                const std::string toString();
                double toNumber();
                bool toBoolean();
                const char* toData();
                size_t getDataLength();
                LuaValueList* toArray();
                LuaValueMap* toMap();

            public:
                static LuaValue* NilValue();
                static LuaValue* BooleanValue(bool value);
                static LuaValue* NumberValue(double value);
                static LuaValue* StringValue(std::string value);
                static LuaValue* DataValue(const char *bytes, size_t length);
                static LuaValue* ArrayValue(LuaValueList value);
                static LuaValue* DictonaryValue(LuaValueMap value);
            };
        }
    }
}

#endif //SAMPLE_LUAVALUE_H
