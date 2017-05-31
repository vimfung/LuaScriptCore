//
// Created by 冯鸿杰 on 2017/5/11.
//

#include "LuaDataExchanger.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaPointer.h"
#include "LuaFunction.h"
#include "StringUtils.h"
#include "LuaTuple.h"
#include "LuaDefined.h"
#include "LuaManagedObject.h"
#include "LuaObjectDescriptor.h"
#include <iostream>
#include <sstream>

using namespace cn::vimfung::luascriptcore;

/**
 记录导入原生层的Lua引用变量表名称
 */
static const char * VarsTableName = "_vars_";

/**
 记录保留的Lua对象变量表名称
 */
static const char * RetainVarsTableName = "_retainVars_";


LuaDataExchanger::LuaDataExchanger(LuaContext *context)
{
    _context = context;
}

LuaValue* LuaDataExchanger::getValue(int stackIndex)
{
    lua_State *state = _context -> getLuaState();
    stackIndex = lua_absindex(state, stackIndex);

    std::string objectId;
    LuaValue *value = NULL;

    int type = lua_type(state, stackIndex);

    switch (type)
    {
        case LUA_TNIL:
        {
            value = LuaValue::NilValue();
            break;
        }
        case LUA_TBOOLEAN:
        {
            value = LuaValue::BooleanValue((bool)lua_toboolean(state, stackIndex));
            break;
        }
        case LUA_TNUMBER:
        {
            value = LuaValue::NumberValue(lua_tonumber(state, stackIndex));
            break;
        }
        case LUA_TSTRING:
        {
            size_t len = 0;
            const char *bytes = lua_tolstring(state, stackIndex, &len);

            if (*(bytes + len) != '\0' || strlen(bytes) != len)
            {
                //为二进制数据流
                value = LuaValue::DataValue(bytes, len);
            }
            else
            {
                //为字符串
                value = LuaValue::StringValue(bytes);
            }

            break;
        }
        case LUA_TTABLE:
        {
            LuaValueMap dictValue;
            LuaValueList arrayValue;
            bool isArray = true;

            lua_pushnil(state);
            while (lua_next(state, stackIndex))
            {
                LuaValue *item = getValue(-1);
                LuaValue *key = getValue(-2);

                if (isArray)
                {
                    if (key -> getType() != LuaValueTypeNumber)
                    {
                        //非数组对象，释放数组
                        isArray = false;
                    }
                    else if (key -> getType() == LuaValueTypeNumber)
                    {
                        int arrayIndex = (int)key->toNumber();
                        if (arrayIndex <= 0)
                        {
                            //非数组对象，释放数组
                            isArray = false;
                        }
                        else if (arrayIndex - 1 != arrayValue.size())
                        {
                            //非数组对象，释放数组
                            isArray = false;
                        }
                        else
                        {
                            arrayValue.push_back(item);
                        }
                    }
                }

                switch (key -> getType())
                {
                    case LuaValueTypeNumber:
                    {
                        std::ostringstream out;
                        out << key->toNumber();
                        dictValue[out.str()] = item;
                        break;
                    }
                    case LuaValueTypeString:
                        dictValue[key->toString()] = item;
                        break;
                    default:
                        if (!isArray)
                        {
                            //如果并非是数组而且key也不是指定类型，则对item进行释放，避免造成内存泄露
                            item -> release();
                        }
                        break;
                }

                key->release();

                lua_pop(state, 1);
            }

            if (isArray)
            {
                value = LuaValue::ArrayValue(arrayValue);
            }
            else
            {
                value = LuaValue::DictonaryValue(dictValue);
            }

            break;
        }
        case LUA_TLIGHTUSERDATA:
        {
            LuaUserdataRef ref = (LuaUserdataRef)lua_topointer(state, stackIndex);
            LuaPointer *pointer = new LuaPointer(ref);
            value = LuaValue::PointerValue(pointer);
            pointer -> release();

            objectId = pointer -> getLinkId();
            break;
        }
        case LUA_TUSERDATA:
        {
            LuaUserdataRef userdataRef = (LuaUserdataRef)lua_touserdata(state, stackIndex);
            void *obj = userdataRef -> value;
            value = LuaValue::ObjectValue((LuaObjectDescriptor *)obj);

            objectId = ((LuaObjectDescriptor *)obj) -> getLinkId();
            break;
        }
        case LUA_TFUNCTION:
        {
            LuaFunction *func = new LuaFunction(_context, stackIndex);
            value = LuaValue::FunctionValue(func);

            objectId = func -> getLinkId();
            break;
        }
        default:
        {
            //默认为nil
            value = LuaValue::NilValue();
            break;
        }
    }

    if (!objectId.empty() && (type == LUA_TTABLE || type == LUA_TUSERDATA || type == LUA_TLIGHTUSERDATA || type == LUA_TFUNCTION))
    {
        //将引用对象放入表中
        beginGetVarsTable();

        lua_pushvalue(state, (int)stackIndex);
        lua_setfield(state, -2, objectId.c_str());

        endGetVarsTable();
    }

    return value;
}

void LuaDataExchanger::pushStack(LuaValue *value)
{
    lua_State *state = _context -> getLuaState();

    //先判断_vars_中是否存在对象，如果存在则直接返回表中对象
    switch (value -> getType())
    {
        case LuaValueTypeInteger:
            lua_pushinteger(state, value -> toInteger());
            break;
        case LuaValueTypeNumber:
            lua_pushnumber(state, value -> toNumber());
            break;
        case LuaValueTypeNil:
            lua_pushnil(state);
            break;
        case LuaValueTypeString:
            lua_pushstring(state, value -> toString().c_str());
            break;
        case LuaValueTypeBoolean:
            lua_pushboolean(state, value -> toBoolean());
            break;
        case LuaValueTypeArray:
        {
            pushStackByTable(value -> toArray());
            break;
        }
        case LuaValueTypeMap:
        {
            pushStackByTable(value -> toMap());
            break;
        }
        case LuaValueTypeData:
        {
            const char *data = value -> toData();
            lua_pushlstring(state, data, value -> getDataLength());
            break;
        }
        case LuaValueTypeObject:
        {
            pushStackByObject(value -> toObject());
            break;
        }
        case LuaValueTypePtr:
        {
            pushStackByObject(value -> toPointer());
            break;
        }
        case LuaValueTypeFunction:
        {
            pushStackByObject(value -> toFunction());
            break;
        }
        case LuaValueTypeTuple:
        {
            value -> toTuple() -> push(_context);
            break;
        }
        default:
            lua_pushnil(state);
            break;
    }
}

void LuaDataExchanger::getLuaObject(LuaObject *object)
{
    if (object)
    {
        std::string linkId;

        LuaValue *value = dynamic_cast<LuaValue *>(object);
        LuaManagedObject *managedObject = dynamic_cast<LuaManagedObject *>(object);
        if (value != NULL)
        {
            switch (value -> getType())
            {
                case LuaValueTypeObject:
                    getLuaObject(value -> toObject());
                    break;
                case LuaValueTypePtr:
                    getLuaObject(value -> toPointer());
                    break;
                case LuaValueTypeFunction:
                    getLuaObject(value -> toFunction());
                    break;
                default:
                    break;
            }
        }
        else if (managedObject != NULL)
        {
            linkId = managedObject -> getLinkId();
        }
        else
        {
            linkId = StringUtils::format("%p", object);
        }

        if (!linkId.empty())
        {
            lua_State *state = _context -> getLuaState();
            
            beginGetVarsTable();
            
            lua_getfield(state, -1, linkId.c_str());
            
            //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
            lua_insert(state, -3);
            
            endGetVarsTable();
        }
    }
}

void LuaDataExchanger::setLuaObject(int stackIndex, const std::string &linkId)
{
    lua_State *state = _context -> getLuaState();

    beginGetVarsTable();

    //放入对象到_vars_表中
    lua_pushvalue(state, stackIndex);
    lua_setfield(state, -2, linkId.c_str());

    endGetVarsTable();
}

void LuaDataExchanger::retainLuaObject(LuaObject *object)
{
    if (object != NULL)
    {
        std::string linkId;

        LuaValue *value = dynamic_cast<LuaValue *>(object);
        LuaManagedObject *managedObject = dynamic_cast<LuaManagedObject *>(object);

        if (value != NULL)
        {
            switch (value -> getType())
            {
                case LuaValueTypeObject:
                    retainLuaObject(value -> toObject());
                    break;
                case LuaValueTypePtr:
                    retainLuaObject(value -> toPointer());
                    break;
                case LuaValueTypeFunction:
                    retainLuaObject(value -> toFunction());
                    break;
                default:
                    break;
            }
        }
        else if (managedObject != NULL)
        {
            linkId = managedObject -> getLinkId();
        }
        else
        {
            linkId = StringUtils::format("%p", object);
        }

        doObjectAction(linkId, LuaObjectActionRetain);
    }
}

void LuaDataExchanger::releaseLuaObject(LuaObject *object)
{
    if (object != NULL)
    {
        std::string linkId;

        LuaValue *value = dynamic_cast<LuaValue *>(object);
        LuaManagedObject *managedObject = dynamic_cast<LuaManagedObject *>(object);

        if (value != NULL)
        {
            switch (value -> getType())
            {
                case LuaValueTypeObject:
                    releaseLuaObject(value -> toObject());
                    break;
                case LuaValueTypePtr:
                    releaseLuaObject(value -> toPointer());
                    break;
                case LuaValueTypeFunction:
                    releaseLuaObject(value -> toFunction());
                    break;
                default:
                    break;
            }
        }
        else if (managedObject != NULL)
        {
            linkId = managedObject -> getLinkId();
        }
        else
        {
            linkId = StringUtils::format("%p", object);
        }

        doObjectAction(linkId, LuaObjectActionRelease);
    }
}

void LuaDataExchanger::beginGetVarsTable()
{
    lua_State *state = _context -> getLuaState();

    lua_getglobal(state, "_G");
    if (!lua_istable(state, -1))
    {
        lua_pop(state, 1);

        lua_newtable(state);

        lua_pushvalue(state, -1);
        lua_setglobal(state, "_G");
    }

    lua_getfield(state, -1, VarsTableName);
    if (lua_isnil(state, -1))
    {
        lua_pop(state, 1);

        //创建引用表
        lua_newtable(state);

        //创建弱引用表元表
        lua_newtable(state);
        lua_pushstring(state, "kv");
        lua_setfield(state, -2, "__mode");
        lua_setmetatable(state, -2);

        //放入全局变量_G中
        lua_pushvalue(state, -1);
        lua_setfield(state, -3, VarsTableName);
    }
}

void LuaDataExchanger::endGetVarsTable()
{
    lua_State *state = _context -> getLuaState();

    //弹出_vars_
    //弹出_G
    lua_pop(state, 2);
}

void LuaDataExchanger::pushStackByObject(LuaManagedObject *object)
{
    //LSCFunction\LSCPointer\NSObject
    lua_State *state = _context -> getLuaState();

    beginGetVarsTable();

    std::string linkId = object -> getLinkId();

    lua_getfield(state, -1, linkId.c_str());
    if (lua_isnil(state, -1))
    {
        //弹出变量
        lua_pop(state, 1);

        //_vars_表中没有对应对象引用，则创建对应引用对象
        object -> push(_context);
    }

    //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
    lua_insert(state, -3);

    endGetVarsTable();
}

void LuaDataExchanger::pushStackByTable(LuaValueList *list)
{
    lua_State *state = _context -> getLuaState();

    lua_newtable(state);

    lua_Integer index = 1;
    for (LuaValueList::iterator it = list -> begin(); it != list -> end(); ++it)
    {
        LuaValue *item = *it;
        pushStack(item);
        lua_rawseti(state, -2, index);

        index ++;
    }
}

void LuaDataExchanger::pushStackByTable(LuaValueMap *map)
{
    lua_State *state = _context -> getLuaState();

    lua_newtable(state);

    for (LuaValueMap::iterator it = map -> begin(); it != map -> end() ; ++it)
    {
        LuaValue *item = it -> second;
        pushStack(item);
        lua_setfield(state, -2, it -> first.c_str());
    }
}

void LuaDataExchanger::doObjectAction(std::string linkId, LuaObjectAction action)
{
    if (!linkId.empty())
    {
        lua_State *state = _context -> getLuaState();

        lua_getglobal(state, "_G");
        if (lua_istable(state, -1))
        {
            lua_getfield(state, -1, VarsTableName);
            if (lua_istable(state, -1))
            {
                //检查对象是否在_vars_表中登记
                lua_getfield(state, -1, linkId.c_str());
                if (!lua_isnil(state, -1))
                {
                    //检查_retainVars_表是否已经记录对象
                    lua_getfield(state, -3, RetainVarsTableName);
                    if (!lua_istable(state, -1))
                    {
                        lua_pop(state, 1);

                        //创建引用表
                        lua_newtable(state);

                        //放入全局变量_G中
                        lua_pushvalue(state, -1);
                        lua_setfield(state, -5, RetainVarsTableName);
                    }

                    switch (action)
                    {
                        case LuaObjectActionRetain:
                        {
                            //保留对象
                            //获取对象
                            lua_getfield(state, -1, linkId.c_str());
                            if (lua_isnil(state, -1)) {
                                lua_pop(state, 1);

                                lua_newtable(state);

                                //初始化引用次数
                                lua_pushnumber(state, 0);
                                lua_setfield(state, -2, "retainCount");

                                lua_pushvalue(state, -3);
                                lua_setfield(state, -2, "object");

                                //将对象放入表中
                                lua_pushvalue(state, -1);
                                lua_setfield(state, -3, linkId.c_str());
                            }

                            //引用次数+1
                            lua_getfield(state, -1, "retainCount");
                            lua_Integer retainCount = lua_tointeger(state, -1);
                            lua_pop(state, 1);

                            lua_pushnumber(state, retainCount + 1);
                            lua_setfield(state, -2, "retainCount");

                            //弹出引用对象
                            lua_pop(state, 1);
                            break;
                        }
                        case LuaObjectActionRelease:
                        {
                            //释放对象
                            //获取对象
                            lua_getfield(state, -1, linkId.c_str());
                            if (!lua_isnil(state, -1)) {
                                //引用次数-1
                                lua_getfield(state, -1, "retainCount");
                                lua_Integer retainCount = lua_tointeger(state, -1);
                                lua_pop(state, 1);

                                if (retainCount - 1 > 0) {
                                    lua_pushnumber(state, retainCount - 1);
                                    lua_setfield(state, -2, "retainCount");
                                }
                                else {
                                    //retainCount<=0时移除对象引用
                                    lua_pushnil(state);
                                    lua_setfield(state, -3, linkId.c_str());
                                }
                            }

                            //弹出引用对象
                            lua_pop(state, 1);
                            break;
                        }
                        default:
                            break;
                    }

                    //弹出_retainVars_
                    lua_pop(state, 1);
                }
                //弹出变量
                lua_pop(state, 1);
            }

            //弹出_vars_
            lua_pop(state, 1);
        }

        //弹出_G
        lua_pop(state, 1);
    }
}
