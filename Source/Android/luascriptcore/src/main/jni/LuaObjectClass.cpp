//
// Created by 冯鸿杰 on 16/9/27.
//

#include <stdio.h>
#include "LuaObjectClass.h"
#include "LuaClassInstance.h"
#include "../../../../../lua-core/src/lua.h"
#include "LuaDefine.h"

/**
 *  对象销毁处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectDestroyHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
    LuaClassInstance *instance = new LuaClassInstance(objectClass -> getContext(), 1);

    //调用对象的destory方法
    instance -> callMethod("destroy", NULL);

    if (objectClass -> getObjectDestroyHandler() != NULL)
    {
        objectClass -> getObjectDestroyHandler() (instance);
    }

    return 0;
}

/**
 *  对象转换为字符串处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectToStringHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LOGI("start call tostring method...");

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
    LuaClassInstance *instance = new LuaClassInstance(objectClass -> getContext(), 1);

    if (objectClass -> getObjectDescriptionHandler() != NULL)
    {
        std::string desc = objectClass -> getObjectDescriptionHandler()(instance);
        lua_pushstring(state, desc.c_str());
    }
    else
    {
        LOGI("call default tostring methos....");
        char strbuf[256] = {0};
        int size = sprintf(strbuf, "[%s object]", objectClass -> getName().c_str());
        strbuf[size] = '\0';
        LOGI("object desc = %s", strbuf);

        lua_pushstring(state, strbuf);
    }

    return 1;
}

/**
 *  创建对象时处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectCreateHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LOGI("call create handler....");

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));

    if (lua_gettop(state) <= 0)
    {
        return 0;
    }

    LOGI("begin create object...");

    //创建对象
    lua_newtable(state);

    //写入一个实例标识
    lua_pushboolean(state, true);
    lua_setfield(state, -2, "_instanceFlag");

    lua_pushvalue(state, 1);
    if (lua_istable(state, -1))
    {
        //设置元表指向类
        lua_setmetatable(state, -2);

        LOGI("end create object...");

        //创建类实例对象
        LuaClassInstance *instance = new LuaClassInstance(objectClass -> getContext(), 1);
        if (objectClass -> getObjectCreatedHandler() != NULL)
        {
            LOGI("callback handler....");

            objectClass -> getObjectCreatedHandler()(instance);
        }

        LOGI("return object...");

        return 1;
    }

    lua_pop(state, 1);

    return 0;
}

/**
 *  子类化
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int subClassHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));

    if (lua_gettop(state) <= 0)
    {
        return 0;
    }

    //获取当前类型的
    lua_pushvalue(state, 1);

    lua_getfield(state, -1, "_instanceFlag");
    if (!lua_isnil(state, -1))
    {
        //实例对象不能调用该方法
        return 0;
    }

    lua_pop(state, 2);

    if (lua_gettop(state) < 2 && !lua_istable(state, 2))
    {
        lua_newtable(state);
    }
    else
    {
        lua_pushvalue(state, 2);
    }

    lua_pushvalue(state, -1);
    lua_setfield(state, -2, "__index");

    int subClassIndex = lua_gettop(state);

    //继承父级gc
    lua_pushvalue(state, 1);
    lua_getfield(state, -1, "__gc");
    lua_setfield(state, subClassIndex, "__gc");
    lua_pop(state, 1);

    //继承父级tostring
    lua_pushvalue(state, 1);
    lua_getfield(state, -1, "__tostring");
    lua_setfield(state, subClassIndex, "__tostring");
    lua_pop(state, 1);

    //设置父类元表
    lua_pushvalue(state, 1);
    lua_setmetatable(state, -2);

    //关联父类
    lua_pushvalue(state, 1);
    lua_setfield(state, -2, "super");

    if (objectClass -> getSubClassHandler() != NULL)
    {
        objectClass -> getSubClassHandler()(objectClass);
    }

    return 1;
}


cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::LuaObjectClass(const std::string &superClassName)
{
    _superClassName = superClassName;
    _classObjectCreatedHandler = NULL;
    _classObjectDescriptionHandler = NULL;
    _classObjectDestroyHandler = NULL;
    _subclassHandler = NULL;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onObjectCreated(LuaClassObjectCreatedHandler handler)
{
    _classObjectCreatedHandler = handler;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onObjectDestroy(LuaClassObjectDestroyHandler handler)
{
    _classObjectDestroyHandler = handler;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onObjectGetDescription (LuaClassObjectGetDescriptionHandler handler)
{
    _classObjectDescriptionHandler = handler;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onSubClass (LuaSubClassHandler handler)
{
    _subclassHandler = handler;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::onRegister(const std::string &name,
                                                                         cn::vimfung::luascriptcore::LuaContext *context)
{
    LOGI("start on class register...");


    cn::vimfung::luascriptcore::LuaModule::onRegister(name, context);

    lua_State *state = context -> getLuaState();
    lua_getglobal(state, name.c_str());

    if (lua_istable(state, -1))
    {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, objectDestroyHandler, 1);
        lua_setfield(state, -2, "__gc");

        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, objectToStringHandler, 1);
        lua_setfield(state, -2, "__tostring");

        if (!_superClassName.empty())
        {
            lua_getglobal(state, _superClassName.c_str());
            if (lua_istable(state, -1))
            {
                //设置父类元表
                lua_setmetatable(state, -2);

                //关联父类
                lua_getglobal(state, _superClassName.c_str());
                lua_setfield(state, -2, "super");
            }
        }
        else
        {
            LOGI("create base method....");

            //Object需要创建对象方法
            lua_pushlightuserdata(state, this);
            lua_pushcclosure(state, objectCreateHandler, 1);
            lua_setfield(state, -2, "create");

            //子类化对象方法
            lua_pushlightuserdata(state, this);
            lua_pushcclosure(state, subClassHandler, 1);
            lua_setfield(state, -2, "subclass");
        }
    }

    lua_pop(state, 1);
}

cn::vimfung::luascriptcore::modules::oo::LuaClassObjectCreatedHandler cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getObjectCreatedHandler()
{
    return _classObjectCreatedHandler;
}

cn::vimfung::luascriptcore::modules::oo::LuaClassObjectDestroyHandler cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getObjectDestroyHandler()
{
    return  _classObjectDestroyHandler;
}

cn::vimfung::luascriptcore::modules::oo::LuaClassObjectGetDescriptionHandler  cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getObjectDescriptionHandler()
{
    return  _classObjectDescriptionHandler;
}

cn::vimfung::luascriptcore::modules::oo::LuaSubClassHandler cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getSubClassHandler()
{
    return _subclassHandler;
}
