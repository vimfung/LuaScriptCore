//
// Created by vimfung on 16/8/23.
//

#include "LuaContext.h"

cn::vimfung::luascriptcore::LuaContext::LuaContext()
{
    _exceptionHandler = NULL;
    _state = luaL_newstate();

    //加载标准库
    luaL_openlibs(_state);
}

cn::vimfung::luascriptcore::LuaContext::~LuaContext()
{
    lua_close(_state);
}

void cn::vimfung::luascriptcore::LuaContext::onException(LuaExceptionHandler handler)
{
    _exceptionHandler = handler;
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::getValueByIndex(int index)
{
    LuaValue *value = NULL;
    switch (lua_type(_state, index)) {
        case LUA_TNIL:
        {
            value = LuaValue::NilValue();
            break;
        }
        case LUA_TBOOLEAN:
        {
            value = LuaValue::BooleanValue((bool)lua_toboolean(_state, index));
            break;
        }
        case LUA_TNUMBER:
        {
            value = LuaValue::NumberValue(lua_tonumber(_state, index));
            break;
        }
        case LUA_TSTRING:
        {
            size_t len = 0;
            const char *bytes = lua_tolstring(_state, index, &len);

            if (strlen(bytes) != len)
            {
                //为二进制数据流
            }
            else
            {
                //为字符串
                value = LuaValue::StringValue(bytes);
            }



//            NSString *strValue =
//            [NSString stringWithCString:lua_tostring(self.state, (int)index)
//            encoding:NSUTF8StringEncoding];
//            if (strValue) {
//                //为NSString
//                value = [LSCValue stringValue:strValue];
//            } else {
//                //为NSData
//                size_t len = 0;
//                const char *bytes = lua_tolstring(self.state, (int)index, &len);
//                NSData *data = [NSData dataWithBytes:bytes length:len];
//
//                value = [LSCValue dataValue:data];
//            }

            break;
        }
//        case LUA_TTABLE: {
//            NSMutableDictionary *dictValue = [NSMutableDictionary dictionary];
//            NSMutableArray *arrayValue = [NSMutableArray array];
//
//            lua_pushnil(self.state);
//            while (lua_next(self.state, -2)) {
//                LSCValue *value = [self getValueByIndex:-1];
//                LSCValue *key = [self getValueByIndex:-2];
//
//                if (arrayValue) {
//                    if (key.valueType != LSCValueTypeNumber) {
//                        //非数组对象，释放数组
//                        arrayValue = nil;
//                    } else if (key.valueType == LSCValueTypeNumber) {
//                        NSInteger index = [[key toNumber] integerValue];
//                        if (index <= 0) {
//                            //非数组对象，释放数组
//                            arrayValue = nil;
//                        } else if (index - 1 != arrayValue.count) {
//                            //非数组对象，释放数组
//                            arrayValue = nil;
//                        } else {
//                            [arrayValue addObject:[value toObject]];
//                        }
//                    }
//                }
//
//                [dictValue setObject:[value toObject] forKey:[key toString]];
//
//                lua_pop(self.state, 1);
//            }
//
//            if (arrayValue) {
//                value = [LSCValue arrayValue:arrayValue];
//            } else {
//                value = [LSCValue dictionaryValue:dictValue];
//            }
//
//            break;
//        }
        default: {
            //默认为nil
            value = LuaValue::NilValue();
            break;
        }
    }

    return value;
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaContext::evalScript(std::string script)
{
    int curTop = lua_gettop(_state);
    int ret = luaL_loadstring(_state, script.c_str()) ||
    lua_pcall(_state, 0, 1, 0);

    bool res = ret == 0;
    if (!res) {

        //错误时触发异常回调
        LuaValue *value = this->getValueByIndex(-1);

        std::string errMessage = value -> toString();
        if (_exceptionHandler != NULL)
        {
            _exceptionHandler (errMessage);
        }

        lua_pop(_state, 1);

    } else {

        if (lua_gettop(_state) > curTop) {

            //有返回值
            LuaValue *value = this -> getValueByIndex(-1);
            lua_pop(_state, 1);

            return value;
        }
    }

    return NULL;
}