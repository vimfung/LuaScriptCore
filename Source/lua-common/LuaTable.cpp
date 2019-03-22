//
//  LuaTable.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/1/18.
//  Copyright © 2019年 冯鸿杰. All rights reserved.
//

#include "LuaTable.hpp"
#include "LuaContext.h"
#include "LuaSession.h"
#include "LuaOperationQueue.h"
#include "LuaDataExchanger.h"
#include "StringUtils.h"

using namespace cn::vimfung::luascriptcore;

LuaTable::LuaTable(LuaValueMap map, std::string exchangeId, LuaContext *context)
    : LuaManagedObject(context)
{
    _valueObject = new LuaValueMap(map);
    _exchangeId = exchangeId;
    _isArray = false;
}

LuaTable::LuaTable(LuaValueList list, std::string exchangeId, LuaContext *context)
    : LuaManagedObject(context)
{
    _valueObject = new LuaValueList(list);
    _exchangeId = exchangeId;
    _isArray = true;
}

LuaTable::~LuaTable()
{
    if (_isArray)
    {
        //对于Table类型需要释放其子对象内存
        LuaValueList *arrayValue = static_cast<LuaValueList *> (_valueObject);
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
    else
    {
        //为字典对象
        LuaValueMap *mapValue = static_cast<LuaValueMap *> (_valueObject);
        if (mapValue != NULL)
        {
            for (LuaValueMap::iterator i = mapValue -> begin(); i != mapValue -> end(); ++i)
            {
                i -> second -> release();
            }
        }
    }
    
    delete[] (char *)_valueObject;
}

bool LuaTable::isArray()
{
    return _isArray;
}

void* LuaTable::getValueObject()
{
    return _valueObject;
}

void LuaTable::setObject(std::string keyPath, LuaValue *object)
{
    if (!_isArray)
    {
        std::deque<std::string> keys = StringUtils::split(keyPath, ".", false);
        setObject((LuaValueMap *)_valueObject, keys, 0, object);
        
        if (getContext() != NULL)
        {
            getContext() -> getOperationQueue() -> performAction([=](){
                
                lua_State *state = getContext() -> getCurrentSession() -> getState();
                getContext() -> getDataExchanger() -> getLuaObject(this);
                
                if (LuaEngineAdapter::type(state, -1) == LUA_TTABLE)
                {
                    //先寻找对应的table对象
                    bool hasExists = true;
                    if (keys.size() > 1)
                    {
                        for (int i = 0; i < keys.size() - 1; i++)
                        {
                            std::string key = keys[i];
                            LuaEngineAdapter::pushString(state, key.c_str());
                            LuaEngineAdapter::rawGet(state, -2);
                            
                            if (LuaEngineAdapter::type(state, -1) == LUA_TTABLE)
                            {
                                //移除前一个table对象
                                LuaEngineAdapter::remove(state, -2);
                            }
                            else
                            {
                                hasExists = false;
                                LuaEngineAdapter::pop(state, 1);
                                break;
                            }
                        }
                    }
                    
                    if (hasExists)
                    {
                        std::string key = keys[keys.size() - 1];
                        LuaEngineAdapter::pushString(state, key.c_str());
                        getContext() -> getDataExchanger() -> pushStack(object);
                        LuaEngineAdapter::rawSet(state, -3);
                    }
                }
                
                LuaEngineAdapter::pop(state, 1);
                
            });
        }
        
    }
}

void LuaTable::setObject(LuaValueMap *map,
                         std::deque<std::string> keys,
                         int keyIndex,
                         LuaValue *object)
{
    if (keyIndex < keys.size())
    {
        std::string key = keys[keyIndex];
        if (keys.size() == keyIndex + 1)
        {
            //最后一个元素
            if (object != NULL)
            {
                object -> retain();
                (*map)[key] = object;
            }
            else
            {
                map -> erase(key);
            }
        }
        else
        {
            LuaValue *value = (*map)[key];
            if (value != NULL && value -> getType() == LuaValueTypeMap)
            {
                LuaValueMap *subMap = value -> toMap();
                setObject(subMap, keys, keyIndex + 1, object);
            }
        }
    }
}

void LuaTable::push(LuaContext *context)
{
    push(context -> getCurrentSession() -> getState(), context -> getOperationQueue());
}

void LuaTable::push(lua_State *state, LuaOperationQueue *queue)
{
    
}
