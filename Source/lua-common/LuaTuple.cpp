//
// Created by 冯鸿杰 on 17/1/19.
//

#include "LuaTuple.h"
#include "LuaValue.h"

cn::vimfung::luascriptcore::LuaTuple::LuaTuple()
{

}

cn::vimfung::luascriptcore::LuaTuple::~LuaTuple()
{
    for (LuaValueList::iterator it = _returnValues.begin(); it != _returnValues.end() ; ++it)
    {
        LuaValue *value = *it;
        value -> release();
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

cn::vimfung::luascriptcore::LuaValue* cn::vimfung::luascriptcore::LuaTuple::getResturValueByIndex(int index)
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