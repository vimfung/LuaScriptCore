//
// Created by 冯鸿杰 on 17/1/19.
//

#include "LuaTuple.h"
#include "LuaValue.h"
#include "LuaNativeClass.hpp"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include <typeinfo>

using namespace cn::vimfung::luascriptcore;

DECLARE_NATIVE_CLASS(LuaTuple);

cn::vimfung::luascriptcore::LuaTuple::LuaTuple()
    :LuaObject()
{

}

LuaTuple::LuaTuple (LuaObjectDecoder *decoder)
    :LuaObject(decoder)
{
    int size = decoder -> readInt32();    
    for (int i = 0; i < size; i++)
    {
        LuaValue *item = dynamic_cast<LuaValue *>(decoder -> readObject());
        _returnValues.push_back(item);
    }
}

cn::vimfung::luascriptcore::LuaTuple::~LuaTuple()
{
    for (LuaValueList::iterator it = _returnValues.begin(); it != _returnValues.end() ; ++it)
    {
        LuaValue *value = *it;
        value -> release();
    }
}

std::string LuaTuple::typeName()
{
    static std::string name = typeid(LuaTuple).name();
    return name;
}

void LuaTuple::serialization (LuaObjectEncoder *encoder)
{
    LuaObject::serialization(encoder);
    
    encoder -> writeInt32((int)_returnValues.size());
    for (LuaValueList::iterator it = _returnValues.begin(); it != _returnValues.end(); ++it)
    {
        LuaValue *value = *it;
        encoder -> writeObject(value);
    }
}

long cn::vimfung::luascriptcore::LuaTuple::count()
{
    return _returnValues.size();
}

void cn::vimfung::luascriptcore::LuaTuple::addReturnValue(LuaValue *value)
{
    _returnValues.push_back(value);
    value -> retain();
}

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaTuple::getReturnValueByIndex(int index)
{
    return _returnValues[index];
}

void cn::vimfung::luascriptcore::LuaTuple::push(LuaContext *context)
{
    for (LuaValueList::iterator it = _returnValues.begin(); it != _returnValues.end() ; ++it)
    {
        LuaValue *value = *it;
        value -> push(context);
    }
}
