//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAVALUE_H
#define SAMPLE_LUAVALUE_H

#include <string>
#include <list>
#include <map>
#include "lua.hpp"
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
                LuaValueTypeInteger = 8,
                LuaValueTypeData = 9
            };

            class LuaValue : public LuaObject
            {
            private:
                LuaValueType _type;
                lua_Integer _intValue;
                bool _booleanValue;
                double _numberValue;
                size_t _bytesLen;
                void *_value;

            public:
                LuaValue ();
                LuaValue (long value);
                LuaValue (bool value);
                LuaValue (double value);
                LuaValue (std::string value);
                LuaValue (const char *bytes, size_t length);
                LuaValue (LuaValueList value);
                LuaValue (LuaValueMap value);
                ~LuaValue();

            public:
                LuaValueType getType();
                long toInteger();
                const std::string toString();
                double toNumber();
                bool toBoolean();
                const char* toData();
                size_t getDataLength();
                LuaValueList* toArray();
                LuaValueMap* toMap();
                void push(lua_State *state);
                void pushValue(lua_State *state, LuaValue *value);
                void pushTable(lua_State *state, LuaValueList *list);
                void pushTable(lua_State *state, LuaValueMap *map);

            public:
                static LuaValue* NilValue();
                static LuaValue* IntegerValue(long value);
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
