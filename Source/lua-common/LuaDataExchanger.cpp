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
#include "LuaSession.h"
#include "LuaEngineAdapter.hpp"
#include "LuaExportTypeDescriptor.hpp"
#include "LuaOperationQueue.h"
#include <iostream>
#include <sstream>
#include <cstring>

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
    LuaValue *value = NULL;

    LuaOperationQueue *queue = _context -> getOperationQueue();
    queue -> performAction([this, &stackIndex, &value, queue](){

        lua_State *state = _context -> getCurrentSession() -> getState();
        stackIndex = LuaEngineAdapter::absIndex(state, stackIndex);

        std::string objectId;

        int type = LuaEngineAdapter::type(state, stackIndex);

        switch (type)
        {
            case LUA_TNIL:
            {
                value = LuaValue::NilValue();
                break;
            }
            case LUA_TBOOLEAN:
            {
                value = LuaValue::BooleanValue((bool)LuaEngineAdapter::toBoolean(state, stackIndex));
                break;
            }
            case LUA_TNUMBER:
            {
                value = LuaValue::NumberValue(LuaEngineAdapter::toNumber(state, stackIndex));
                break;
            }
            case LUA_TSTRING:
            {
                size_t len = 0;
                const char *bytes = LuaEngineAdapter::toLString(state, stackIndex, &len);

                //fixed: 解决部分二进制数据流长度无法判断时，改用UTF-8编码判断
                //所有非UTF-8编码字符串均转换为二进制数据
                if (*(bytes + len) != '\0' || std::strlen(bytes) != len || !StringUtils::isUTF8String(bytes))
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
                //判断是否为类型
                LuaEngineAdapter::getField(state, stackIndex, "_nativeType");
                if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
                {
                    //为导出类型
                    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, -1);
                    value = new LuaValue(typeDescriptor);

                    LuaEngineAdapter::pop(state, 1);
                }
                else
                {
                    //出栈前一结果
                    LuaEngineAdapter::pop(state, 1);

                    const void *userdata = LuaEngineAdapter::toPointer(state, -1);
                    objectId = StringUtils::format("%p", userdata);

                    LuaValueMap dictValue;
                    LuaValueList arrayValue;
                    bool isArray = true;

                    LuaEngineAdapter::pushNil(state);
                    while (LuaEngineAdapter::next(state, stackIndex))
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
                            else
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

                        LuaEngineAdapter::pop(state, 1);
                    }

                    if (isArray && arrayValue.size() > 0)
                    {
                        value = new LuaValue(arrayValue, objectId);
                    }
                    else
                    {
                        value = new LuaValue(dictValue, objectId);
                    }
                }

                break;
            }
            case LUA_TLIGHTUSERDATA:
            {
                LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::toPointer(state, stackIndex);
                LuaPointer *pointer = new LuaPointer(_context, ref);
                value = LuaValue::PointerValue(pointer);
                pointer -> release();

                objectId = pointer -> getExchangeId();
                break;
            }
            case LUA_TUSERDATA:
            {
                LuaUserdataRef userdataRef = (LuaUserdataRef)LuaEngineAdapter::toUserdata(state, stackIndex);
                void *obj = userdataRef -> value;
                value = LuaValue::ObjectValue((LuaObjectDescriptor *)obj);

                objectId = ((LuaObjectDescriptor *)obj) -> getExchangeId();
                break;
            }
            case LUA_TFUNCTION:
            {
                LuaFunction *func = new LuaFunction(_context, stackIndex);
                value = LuaValue::FunctionValue(func);

                objectId = func -> getExchangeId();
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
            beginGetVarsTable(state, queue);

            LuaEngineAdapter::pushValue(state, stackIndex);
            LuaEngineAdapter::setField(state, -2, objectId.c_str());

            endGetVarsTable(state, queue);
        }

    });

    return value;
}

void LuaDataExchanger::pushStack(LuaValue *value)
{
    pushStack(value,
              _context -> getCurrentSession() -> getState(),
              _context -> getOperationQueue());
}

void LuaDataExchanger::pushStack(LuaValue *value, lua_State *state, LuaOperationQueue *queue)
{
    auto handler = [this, value, state, queue](){

        //先判断_vars_中是否存在对象，如果存在则直接返回表中对象
        switch (value -> getType())
        {
            case LuaValueTypeInteger:
                LuaEngineAdapter::pushInteger(state, value -> toInteger());
                break;
            case LuaValueTypeNumber:
                LuaEngineAdapter::pushNumber(state, value -> toNumber());
                break;
            case LuaValueTypeNil:
                LuaEngineAdapter::pushNil(state);
                break;
            case LuaValueTypeString:
                LuaEngineAdapter::pushString(state, value -> toString().c_str());
                break;
            case LuaValueTypeBoolean:
                LuaEngineAdapter::pushBoolean(state, value -> toBoolean());
                break;
            case LuaValueTypeArray:
            {
                pushStackByTable(value -> toArray(), value -> tableId(), state, queue);
                break;
            }
            case LuaValueTypeMap:
            {
                pushStackByTable(value -> toMap(), value -> tableId(), state, queue);
                break;
            }
            case LuaValueTypeData:
            {
                const char *data = value -> toData();
                LuaEngineAdapter::pushString(state, data, value -> getDataLength());
                break;
            }
            case LuaValueTypeObject:
            {
                pushStackByObject(value -> toObject(), state, queue);
                break;
            }
            case LuaValueTypePtr:
            {
                pushStackByObject(value -> toPointer(), state, queue);
                break;
            }
            case LuaValueTypeFunction:
            {
                pushStackByObject(value -> toFunction(), state, queue);
                break;
            }
            case LuaValueTypeTuple:
            {
                value -> toTuple() -> push(_context);
                break;
            }
            default:
                LuaEngineAdapter::pushNil(state);
                break;
        }

    };

    if (queue != NULL)
    {
        queue -> performAction(handler);
    }
    else
    {
        handler ();
    }

}

void LuaDataExchanger::getLuaObject(LuaObject *object)
{
    getLuaObject(object, _context -> getCurrentSession() -> getState(), _context -> getOperationQueue());
}

void LuaDataExchanger::getLuaObject(LuaObject *object, lua_State *state, LuaOperationQueue *queue)
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
                    getLuaObject(value -> toObject(), state, queue);
                    break;
                case LuaValueTypePtr:
                    getLuaObject(value -> toPointer(), state, queue);
                    break;
                case LuaValueTypeFunction:
                    getLuaObject(value -> toFunction(), state, queue);
                    break;
                case LuaValueTypeMap:
                case LuaValueTypeArray:
                    //为Map或者Array时直接读取table标志
                    linkId = value -> tableId();
                    break;
                default:
                    break;
            }
        }
        else if (managedObject != NULL)
        {
            linkId = managedObject -> getExchangeId();
        }
        else
        {
            linkId = StringUtils::format("%p", object);
        }

        if (!linkId.empty())
        {

            beginGetVarsTable(state, queue);

            auto handler = [this, linkId, state](){

                LuaEngineAdapter::getField(state, -1, linkId.c_str());

                //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                LuaEngineAdapter::insert(state, -3);

            };

            if (queue != NULL)
            {
                queue -> performAction(handler);
            }
            else
            {
                handler ();
            }

            endGetVarsTable(state, queue);
        }
    }
}

void LuaDataExchanger::setLuaObject(int stackIndex, std::string const& linkId)
{
    LuaOperationQueue *queue = _context -> getOperationQueue();
    queue -> performAction([this, &stackIndex, &linkId, queue](){

        lua_State *state = _context -> getCurrentSession() -> getState();

        stackIndex = LuaEngineAdapter::absIndex(state, stackIndex);

        beginGetVarsTable(state, queue);

        //放入对象到_vars_表中
        LuaEngineAdapter::pushValue(state, stackIndex);
        LuaEngineAdapter::setField(state, -2, linkId.c_str());

        endGetVarsTable(state, queue);

    });
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
                    return;
                case LuaValueTypePtr:
                    retainLuaObject(value -> toPointer());
                    return;
                case LuaValueTypeFunction:
                    retainLuaObject(value -> toFunction());
                    return;
                default:
                    break;
            }
        }
        else if (managedObject != NULL)
        {
            linkId = managedObject -> getExchangeId();
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
                    return;
                case LuaValueTypePtr:
                    releaseLuaObject(value -> toPointer());
                    return;
                case LuaValueTypeFunction:
                    releaseLuaObject(value -> toFunction());
                    return;
                default:
                    break;
            }
        }
        else if (managedObject != NULL)
        {
            linkId = managedObject -> getExchangeId();
        }
        else
        {
            linkId = StringUtils::format("%p", object);
        }

        doObjectAction(linkId, LuaObjectActionRelease);
    }
}

void LuaDataExchanger::beginGetVarsTable(lua_State *state, LuaOperationQueue *queue)
{
    auto handler = [this, state](){

        LuaEngineAdapter::getGlobal(state, "_G");
        if (!LuaEngineAdapter::isTable(state, -1))
        {
            LuaEngineAdapter::pop(state, 1);

            LuaEngineAdapter::newTable(state);

            LuaEngineAdapter::pushValue(state, -1);
            LuaEngineAdapter::setGlobal(state, "_G");
        }

        LuaEngineAdapter::getField(state, -1, VarsTableName);
        if (LuaEngineAdapter::isNil(state, -1))
        {
            LuaEngineAdapter::pop(state, 1);

            //创建引用表
            LuaEngineAdapter::newTable(state);

            //创建弱引用表元表
            LuaEngineAdapter::newTable(state);
            LuaEngineAdapter::pushString(state, "kv");
            LuaEngineAdapter::setField(state, -2, "__mode");
            LuaEngineAdapter::setMetatable(state, -2);

            //放入全局变量_G中
            LuaEngineAdapter::pushValue(state, -1);
            LuaEngineAdapter::setField(state, -3, VarsTableName);
        }

    };

    if (queue != NULL)
    {
        queue -> performAction(handler);
    }
    else
    {
        handler ();
    }

}

void LuaDataExchanger::endGetVarsTable(lua_State *state, LuaOperationQueue *queue)
{
    auto handler = [this, state](){

        //弹出_vars_
        //弹出_G
        LuaEngineAdapter::pop(state, 2);

    };

    if (queue != NULL)
    {
        queue -> performAction(handler);
    }
    else
    {
        handler ();
    }
}

void LuaDataExchanger::pushStackByObject(LuaManagedObject *object, lua_State *state, LuaOperationQueue *queue)
{
    //LuaFunction\LuaPointer\ObjectDescription
    auto handler = [this, object, state, queue](){

        beginGetVarsTable(state, queue);

        std::string linkId = object -> getExchangeId();
        
        //先从表中查找关联对象
        LuaEngineAdapter::getField(state, -1, linkId.c_str());
        if (LuaEngineAdapter::isNil(state, -1))
        {
            //弹出变量
            LuaEngineAdapter::pop(state, 1);
            
            //_vars_表中没有对应对象引用，则创建对应引用对象
            object -> push(state, queue);
            
            //放入_vars_表中，修复如果对象从未在lua回调回来时，无法找到对应对象问题。
            LuaEngineAdapter::pushValue(state, -1);
            LuaEngineAdapter::setField(state, -3, linkId.c_str());
        }
        

        //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
        LuaEngineAdapter::insert(state, -3);

        endGetVarsTable(state, queue);

    };

    if (queue != NULL)
    {
        queue -> performAction(handler);
    }
    else
    {
        handler();
    }

}

void LuaDataExchanger::pushStackByTable(LuaValueList *list, std::string tableId, lua_State *state, LuaOperationQueue *queue)
{
    bool exists = true;
    if (!tableId.empty())
    {
        auto handler = [&](){

            beginGetVarsTable(state, queue);

            //先从表中查找关联对象
            LuaEngineAdapter::getField(state, -1, tableId.c_str());
            if (LuaEngineAdapter::isNil(state, -1))
            {
                //弹出变量
                LuaEngineAdapter::pop(state, 1);
                exists = false;
            }
            else
            {
                //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                LuaEngineAdapter::insert(state, -3);
            }

            endGetVarsTable(state, queue);

        };

        if (queue != NULL)
        {
            queue -> performAction(handler);
        }
        else
        {
            handler ();
        }
    }
    else
    {
        exists = false;
    }

    if (!exists)
    {
        auto handler = [this, list, state, queue](){

            LuaEngineAdapter::newTable(state);

            int index = 1;
            for (LuaValueList::iterator it = list -> begin(); it != list -> end(); ++it)
            {
                LuaValue *item = *it;
                pushStack(item, state, queue);
                LuaEngineAdapter::rawSetI(state, -2, index);

                index ++;
            }

        };

        if (queue != NULL)
        {
            queue -> performAction(handler);
        }
        else
        {
            handler ();
        }
    }

}

void LuaDataExchanger::pushStackByTable(LuaValueMap *map, std::string tableId, lua_State *state, LuaOperationQueue *queue)
{
    bool exists = true;
    if (!tableId.empty())
    {
        auto handler = [&](){

            beginGetVarsTable(state, queue);

            //先从表中查找关联对象
            LuaEngineAdapter::getField(state, -1, tableId.c_str());
            if (LuaEngineAdapter::isNil(state, -1))
            {
                //弹出变量
                LuaEngineAdapter::pop(state, 1);
                exists = false;
            }
            else
            {
                //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                LuaEngineAdapter::insert(state, -3);
            }

            endGetVarsTable(state, queue);

        };

        if (queue != NULL)
        {
            queue -> performAction(handler);
        }
        else
        {
            handler ();
        }
    }
    else
    {
        exists = false;
    }

    if (!exists)
    {
        //不存在talbeId对应的变量则作为新对象入栈
        auto handler = [this, map, state, queue](){

            LuaEngineAdapter::newTable(state);

            for (LuaValueMap::iterator it = map -> begin(); it != map -> end() ; ++it)
            {
                LuaValue *item = it -> second;
                pushStack(item, state, queue);
                LuaEngineAdapter::setField(state, -2, it -> first.c_str());
            }

        };

        if (queue != NULL)
        {
            queue -> performAction(handler);
        }
        else
        {
            handler ();
        }
    }

}

void LuaDataExchanger::doObjectAction(std::string const& linkId, LuaObjectAction action)
{
    _context -> getOperationQueue() -> performAction([this, linkId, action](){

        if (!linkId.empty())
        {
            lua_State *state = _context -> getCurrentSession() -> getState();

            LuaEngineAdapter::getGlobal(state, "_G");
            if (LuaEngineAdapter::isTable(state, -1))
            {
                LuaEngineAdapter::getField(state, -1, VarsTableName);
                if (LuaEngineAdapter::isTable(state, -1))
                {
                    //检查对象是否在_vars_表中登记
                    LuaEngineAdapter::getField(state, -1, linkId.c_str());
                    if (!LuaEngineAdapter::isNil(state, -1))
                    {
                        //检查_retainVars_表是否已经记录对象
                        LuaEngineAdapter::getField(state, -3, RetainVarsTableName);
                        if (!LuaEngineAdapter::isTable(state, -1))
                        {
                            LuaEngineAdapter::pop(state, 1);

                            //创建引用表
                            LuaEngineAdapter::newTable(state);

                            //放入全局变量_G中
                            LuaEngineAdapter::pushValue(state, -1);
                            LuaEngineAdapter::setField(state, -5, RetainVarsTableName);
                        }

                        switch (action)
                        {
                            case LuaObjectActionRetain:
                            {
                                //保留对象
                                //获取对象
                                LuaEngineAdapter::getField(state, -1, linkId.c_str());
                                if (LuaEngineAdapter::isNil(state, -1))
                                {
                                    LuaEngineAdapter::pop(state, 1);

                                    LuaEngineAdapter::newTable(state);

                                    //初始化引用次数
                                    LuaEngineAdapter::pushNumber(state, 0);
                                    LuaEngineAdapter::setField(state, -2, "retainCount");

                                    LuaEngineAdapter::pushValue(state, -3);
                                    LuaEngineAdapter::setField(state, -2, "object");

                                    //将对象放入表中
                                    LuaEngineAdapter::pushValue(state, -1);
                                    LuaEngineAdapter::setField(state, -3, linkId.c_str());
                                }

                                //引用次数+1
                                LuaEngineAdapter::getField(state, -1, "retainCount");
                                lua_Integer retainCount = LuaEngineAdapter::toInteger(state, -1);
                                LuaEngineAdapter::pop(state, 1);

                                LuaEngineAdapter::pushNumber(state, retainCount + 1);
                                LuaEngineAdapter::setField(state, -2, "retainCount");

                                //弹出引用对象
                                LuaEngineAdapter::pop(state, 1);
                                break;
                            }
                            case LuaObjectActionRelease:
                            {
                                //释放对象
                                //获取对象
                                LuaEngineAdapter::getField(state, -1, linkId.c_str());
                                if (!LuaEngineAdapter::isNil(state, -1))
                                {
                                    //引用次数-1
                                    LuaEngineAdapter::getField(state, -1, "retainCount");
                                    lua_Integer retainCount = LuaEngineAdapter::toInteger(state, -1);
                                    LuaEngineAdapter::pop(state, 1);

                                    if (retainCount - 1 > 0)
                                    {
                                        LuaEngineAdapter::pushNumber(state, retainCount - 1);
                                        LuaEngineAdapter::setField(state, -2, "retainCount");
                                    }
                                    else
                                    {
                                        //retainCount<=0时移除对象引用
                                        LuaEngineAdapter::pushNil(state);
                                        LuaEngineAdapter::setField(state, -3, linkId.c_str());
                                    }
                                }

                                //弹出引用对象
                                LuaEngineAdapter::pop(state, 1);
                                break;
                            }
                            default:
                                break;
                        }

                        //弹出_retainVars_
                        LuaEngineAdapter::pop(state, 1);
                    }
                    //弹出变量
                    LuaEngineAdapter::pop(state, 1);
                }

                //弹出_vars_
                LuaEngineAdapter::pop(state, 1);
            }

            //弹出_G
            LuaEngineAdapter::pop(state, 1);
        }

    });

}

void LuaDataExchanger::clearObject(LuaManagedObject *object)
{
    LuaOperationQueue *queue = _context -> getOperationQueue();
    lua_State *state = _context -> getCurrentSession() -> getState();

    /*
     * fixed：清除_vars_表中持有的对象，虽然_vars_是弱引用表，但是对象释放后器kv依然会保留在表中，因此在对象释放时需要通知该表将对应的key置空，
     * 否则当有新对象分配到该内存地址的时候就会对某些业务操作造成影响
     * */
    beginGetVarsTable(state, queue);

    _context -> getOperationQueue() -> performAction([this, object, state, queue](){

        const char *linkId = object -> getExchangeId().c_str();

        LuaEngineAdapter::getField(state, -1, linkId);
        if (!LuaEngineAdapter::isNil(state, -1))
        {
            LuaEngineAdapter::pushNil(state);
            LuaEngineAdapter::setField(state, -3, linkId);
        }

        LuaEngineAdapter::pop(state, 1);

    });

    endGetVarsTable(state, queue);
}
