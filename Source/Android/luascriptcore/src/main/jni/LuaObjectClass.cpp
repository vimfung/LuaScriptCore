//
// Created by 冯鸿杰 on 16/9/27.
//

#include <stdio.h>
#include <ctype.h>
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
    LuaClassInstance *instance = new LuaClassInstance(objectClass, 1);

    //调用对象的destory方法
    instance -> callMethod("destroy", NULL);

    if (objectClass -> getObjectDestroyHandler() != NULL)
    {
        objectClass -> getObjectDestroyHandler() (instance);
    }

    instance -> release();

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

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
    LuaClassInstance *instance = new LuaClassInstance(objectClass, 1);

    if (objectClass -> getObjectDescriptionHandler() != NULL)
    {
        std::string desc = objectClass -> getObjectDescriptionHandler()(instance);
        lua_pushstring(state, desc.c_str());
    }
    else
    {
        char strbuf[256] = {0};
        int size = sprintf(strbuf, "[%s object]", objectClass -> getName().c_str());
        strbuf[size] = '\0';

        lua_pushstring(state, strbuf);
    }

    instance -> release();

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

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));

    LOGI("Create Object Class = %s", objectClass-> getName().c_str());

    if (lua_gettop(state) <= 0)
    {
        return 0;
    }

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

        //创建类实例对象
        LuaClassInstance *instance = new LuaClassInstance(objectClass, lua_gettop(state));
        if (objectClass -> getObjectCreatedHandler() != NULL)
        {
            objectClass -> getObjectCreatedHandler()(instance);
        }
        instance -> release();

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

/**
 * 实例方法路由处理
 *
 * @param state lua状态机
 *
 * @return 返回值数量
 */
static int instanceMethodRouteHandler(lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
    const char *methodName = lua_tostring(state, lua_upvalueindex(2));

    LuaInstanceMethodHandler handler = objectClass -> getInstanceMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaContext *context = objectClass -> getContext();

        int top = lua_gettop(state);
        cn::vimfung::luascriptcore::LuaArgumentList args;
        for (int i = 2; i < top; i++)
        {
            cn::vimfung::luascriptcore::LuaValue *value = context -> getValueByIndex(i);
            args.push_back(value);
        }

        LuaClassInstance *instance = new LuaClassInstance(objectClass, 1);
        cn::vimfung::luascriptcore::LuaValue *retValue = handler (instance, methodName, args);
        if (retValue != NULL)
        {
            retValue -> push(state);
            retValue -> release();
        }
        instance -> release();

        //释放参数内存
        for (cn::vimfung::luascriptcore::LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            cn::vimfung::luascriptcore::LuaValue *item = *it;
            item -> release();
        }
    }

    return 1;
}

/**
 * 实例设置器路由处理
 *
 * @param state lua状态机
 *
 * @return 返回值数量
 */
static int instanceSetterRouteHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
    const char *fieldName = lua_tostring(state, lua_upvalueindex(2));

    LuaInstanceSetterHandler handler = objectClass -> getInstanceSetterHandler(fieldName);
    if (handler != NULL)
    {
        LuaContext *context = objectClass -> getContext();

        cn::vimfung::luascriptcore::LuaValue *value = NULL;
        int top = lua_gettop(state);
        if (top > 0)
        {
            value = context -> getValueByIndex(2);
        }
        else
        {
            value = new cn::vimfung::luascriptcore::LuaValue();
        }

        LuaClassInstance *instance = new LuaClassInstance(objectClass, 1);
        handler (instance, fieldName, value);
        instance -> release();

        //释放参数内存
        value -> release();
    }

    return 0;
}

/**
 * 实例获取器路由处理
 *
 * @param state lua状态机
 *
 * @return 返回值数量
 */
static int instanceGetterRouteHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
    const char *fieldName = lua_tostring(state, lua_upvalueindex(2));

    LuaInstanceGetterHandler handler = objectClass -> getGetterHandler(fieldName);
    if (handler != NULL)
    {
        LuaClassInstance *instance = new LuaClassInstance(objectClass, 1);
        cn::vimfung::luascriptcore::LuaValue *retValue = handler (instance, fieldName);
        if (retValue != NULL)
        {
            retValue -> push(state);
            retValue -> release();
        }
        else
        {
            lua_pushnil(state);
        }
        instance -> release();
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

        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, objectCreateHandler, 1);
        lua_setfield(state, -2, "create");

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

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::registerInstanceField(std::string fieldName, LuaInstanceGetterHandler getterHandler, LuaInstanceSetterHandler setterHandler)
{
    //与iOS中的属性getter和setter方法保持一致, getter直接是属性名称,setter则需要将属性首字母大写并在前面加上set
    char upperCStr[2] = {0};
    upperCStr[0] = toupper(fieldName[0]);
    std::string upperStr = upperCStr;
    std::string fieldNameStr = fieldName.c_str() + 1;
    std::string setterMethodName = "set" + upperStr + fieldNameStr;

    lua_State *state = getContext() -> getLuaState();
    lua_getglobal(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, setterMethodName.c_str());
        if (!lua_isnil(state, -1))
        {
            return;
        }
        lua_pop(state, 1);

        lua_getfield(state, -1, fieldName.c_str());
        if (!lua_isnil(state, -1))
        {
            return;
        }
        lua_pop(state, 1);

        //设置Setter方法
        lua_pushlightuserdata(state, this);
        lua_pushstring(state, fieldName.c_str());
        lua_pushcclosure(state, instanceSetterRouteHandler, 2);

        lua_setfield(state, -2, setterMethodName.c_str());

        _instanceSetterMap[fieldName] = setterHandler;

        //注册Getter方法
        lua_pushlightuserdata(state, this);
        lua_pushstring(state, fieldName.c_str());
        lua_pushcclosure(state, instanceGetterRouteHandler, 2);

        lua_setfield(state, -2, fieldName.c_str());

        _instanceGetterMap[fieldName] = getterHandler;
    }
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::registerInstanceMethod(std::string methodName, LuaInstanceMethodHandler handler)
{
    lua_State *state = getContext() -> getLuaState();
    lua_getglobal(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, methodName.c_str());
        if (lua_isnil(state, -1))
        {
            //尚未注册
            lua_pop(state, 1);

            lua_pushlightuserdata(state, this);
            lua_pushstring(state, methodName.c_str());
            lua_pushcclosure(state, instanceMethodRouteHandler, 2);

            lua_setfield(state, -2, methodName.c_str());

            _instanceMethodMap[methodName] = handler;
        }

    }
}

cn::vimfung::luascriptcore::modules::oo::LuaInstanceMethodHandler cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getInstanceMethodHandler(std::string methodName)
{
    LuaInstanceMethodMap::iterator it =  _instanceMethodMap.find(methodName.c_str());
    if (it != _instanceMethodMap.end())
    {
        return it -> second;
    }

    return NULL;
}

cn::vimfung::luascriptcore::modules::oo::LuaInstanceSetterHandler cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getInstanceSetterHandler(std::string fieldName)
{
    LuaInstanceSetterMap::iterator it =  _instanceSetterMap.find(fieldName.c_str());
    if (it != _instanceSetterMap.end())
    {
        return it -> second;
    }

    return NULL;
}

cn::vimfung::luascriptcore::modules::oo::LuaInstanceGetterHandler cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getGetterHandler(std::string fieldName)
{
    LuaInstanceGetterMap::iterator it =  _instanceGetterMap.find(fieldName.c_str());
    if (it != _instanceGetterMap.end())
    {
        return it -> second;
    }

    return NULL;
}