//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAVALUE_H
#define SAMPLE_LUAVALUE_H

#include <string>
#include "LuaObject.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            enum LuaValueType
            {
                LuaValueTypeNil = 0,
                LuaValueTypeNumber = 1,
                LuaValueTypeBoolean = 2,
                LuaValueTypeString = 3,
                LuaValueTypeTable = 4,
                LuaValueTypeFunction = 5,
                LuaValueTypeThread = 6,
                LuaValueTypeUserData = 7,
                LuaValueTypeInteger = 8,
                LuaValueTypeData = 9
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
                ~LuaValue();

            public:
                LuaValueType getType();
                const std::string toString();
                double toNumber();

            public:
                static LuaValue* NilValue();
                static LuaValue* BooleanValue(bool value);
                static LuaValue* NumberValue(double value);
                static LuaValue* StringValue(std::string value);
                static LuaValue* DataValue(const char *bytes, size_t length);
            };
        }
    }
}

#endif //SAMPLE_LUAVALUE_H
