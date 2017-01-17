//
// Created by 冯鸿杰 on 16/9/27.
//

#include <limits>
#include <stdio.h>
#include <ctype.h>
#include "LuaObjectClass.h"
#include "LuaObjectInstanceDescriptor.h"

/**
 * 实例种子，参与实例索引的生成，每次创建实例，该值会自增.
 */
static int InstanceSeed = 0;

/**
 * 实例引用表名称
 */
static std::string InstanceRefsTableName = "_instanceRefs_";

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

    if (objectClass -> getObjectDestroyHandler() != NULL)
    {
        objectClass -> getObjectDestroyHandler() (objectClass);
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

    std::string desc;

    int type = lua_type(state, 1);

    //由于加入了实例的super对象，因此需要根据不同类型进行不同输出。since ver 1.3
    switch (type)
    {
        case LUA_TUSERDATA:
        {
            LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(1));
            if (objectClass -> getObjectDescriptionHandler() != NULL)
            {
                desc = objectClass -> getObjectDescriptionHandler()(objectClass);
            }

            if (desc.empty())
            {
                char strbuf[256] = {0};
                int size = sprintf(strbuf, "[%s object]", objectClass -> getName().c_str());
                strbuf[size] = '\0';

                lua_pushstring(state, strbuf);
            }
            else
            {
                lua_pushstring(state, desc.c_str());
            }
            break;
        }
        case LUA_TTABLE:
        {
            lua_pushstring(state, "<SuperClass Type>");
            break;
        }
        default:
        {
            lua_pushstring(state, "<Unknown Type>");
            break;
        }
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

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(2));

    if (objectClass -> getObjectCreatedHandler() != NULL)
    {
        objectClass -> getObjectCreatedHandler() (objectClass);
    }

    return 1;
}

/**
 * 更新类字段时触发
 *
 * @param state lua状态机
 *
 * @return 参数数量
 */
static int objectNewIndexHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore::modules::oo;

    //限于当前无法判断所定义的方法是使用.或:定义，因此对添加的属性或者方法统一添加到类表和实例元表中。
    lua_pushvalue(state, 2);
    lua_pushvalue(state, 3);
    lua_rawset(state, 1);

    //查找实例元表进行添加
    lua_getfield(state, 1, "_nativeClass");
    LuaObjectClass *objectClass = (LuaObjectClass *)lua_topointer(state, -1);
    luaL_getmetatable(state, objectClass -> getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_pushvalue(state, 2);
        lua_pushvalue(state, 3);
        lua_rawset(state, -3);
    }
    lua_pop(state, 1);

    return 0;
}

/**
 实例对象更新索引处理

 @param state 状态机
 @return 参数数量
 */
static int instanceNewIndexHandler (lua_State *state)
{
    //先找到实例对象的元表，向元表添加属性
    lua_getmetatable(state, 1);
    if (lua_istable(state, -1))
    {
        lua_pushvalue(state, 2);
        lua_pushvalue(state, 3);
        lua_rawset(state, -3);
    }

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

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(2));

    if (lua_gettop(state) == 0)
    {
        return 0;
    }

    const char *subclassName = luaL_checkstring(state, 1);

    if (objectClass -> getSubClassHandler() != NULL)
    {
        objectClass -> getSubClassHandler()(objectClass, subclassName);
    }

    return 0;
}

/**
 判断是否是该类型的子类

 @param state 状态机
 @return 参数数量
 */
static int subclassOfHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore::modules::oo;

    if (lua_gettop(state) == 0)
    {
        lua_pushboolean(state, false);
        return 1;
    }

    LuaObjectClass *objectClass = (LuaObjectClass *)lua_touserdata(state, lua_upvalueindex(2));

    if (lua_type(state, 1) == LUA_TTABLE)
    {
        lua_getfield(state, 1, "_nativeClass");
        if (lua_type(state, -1) == LUA_TLIGHTUSERDATA)
        {
            LuaObjectClass *checkClass = (LuaObjectClass *)lua_touserdata(state, -1);

            bool flag = objectClass -> subclassOf(checkClass);
            lua_pushboolean(state, flag);

            return 1;
        }
    }

    lua_pushboolean(state, false);
    return 1;
}

/**
 判断是否是该类型的实例对象

 @param state 状态机
 @return 参数数量
 */
static int instanceOfHandler (lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;

    if (lua_gettop(state) < 2)
    {
        lua_pushboolean(state, false);
        return 1;
    }

    //表示有实例对象传入
    LuaUserdataRef ref = (LuaUserdataRef)lua_touserdata(state, 1);
    LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)ref -> value;

    if (lua_type(state, 2) == LUA_TTABLE)
    {
        lua_getfield(state, 2, "_nativeClass");
        if (lua_type(state, -1) == LUA_TLIGHTUSERDATA)
        {
            LuaObjectClass *objectClass = (LuaObjectClass *)lua_topointer(state, -1);

            bool flag = objDesc -> instanceOf(objectClass);
            lua_pushboolean(state, flag);
            return 1;
        }
    }

    lua_pushboolean(state, false);
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
    std::string methodName = lua_tostring(state, lua_upvalueindex(2));

    if (lua_type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + methodName + " method error : missing self parameter, please call by instance:methodName(param)";
        objectClass -> getContext() -> raiseException(errMsg);

        //回收内存
        lua_gc(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    cn::vimfung::luascriptcore::LuaUserdataRef instance = (cn::vimfung::luascriptcore::LuaUserdataRef)lua_touserdata(state, 1);

    LuaInstanceMethodHandler handler = objectClass -> getInstanceMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaContext *context = objectClass -> getContext();

        int top = lua_gettop(state);
        cn::vimfung::luascriptcore::LuaArgumentList args;
        for (int i = 2; i <= top; i++)
        {
            cn::vimfung::luascriptcore::LuaValue *value = context -> getValueByIndex(i);
            args.push_back(value);
        }

        cn::vimfung::luascriptcore::LuaValue *retValue = handler (instance, objectClass, methodName, args);

        if (retValue != NULL)
        {
            //释放返回值
            retValue -> push(context);
            retValue -> release();
        }

        //释放参数内存
        for (cn::vimfung::luascriptcore::LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            cn::vimfung::luascriptcore::LuaValue *item = *it;
            item -> release();
        }
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

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
    std::string fieldName = lua_tostring(state, lua_upvalueindex(2));

    if (lua_type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        objectClass -> getContext() -> raiseException(errMsg);

        //回收内存
        lua_gc(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    cn::vimfung::luascriptcore::LuaUserdataRef instance = (cn::vimfung::luascriptcore::LuaUserdataRef)lua_touserdata(state, 1);

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

        handler (instance, objectClass, fieldName, value);

        //释放参数内存
        value -> release();
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

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
    std::string fieldName = lua_tostring(state, lua_upvalueindex(2));

    if (lua_type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        objectClass -> getContext() -> raiseException(errMsg);

        //回收内存
        lua_gc(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    cn::vimfung::luascriptcore::LuaUserdataRef instance = (cn::vimfung::luascriptcore::LuaUserdataRef)lua_touserdata(state, 1);

    LuaInstanceGetterHandler handler = objectClass -> getGetterHandler(fieldName);
    if (handler != NULL)
    {
        cn::vimfung::luascriptcore::LuaValue *retValue = handler (instance, objectClass, fieldName);
        if (retValue != NULL)
        {
            retValue -> push(objectClass -> getContext());
            retValue -> release();
        }
        else
        {
            lua_pushnil(state);
        }
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

    return 1;
}

cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::LuaObjectClass(LuaObjectClass *superClass)
{
    _superClass = superClass;
    _classObjectCreatedHandler = NULL;
    _classObjectDescriptionHandler = NULL;
    _classObjectDestroyHandler = NULL;
    _subclassHandler = NULL;
}

//cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::LuaObjectClass(const std::string &superClassName)
//{
//    _superClassName = superClassName;
//    _classObjectCreatedHandler = NULL;
//    _classObjectDescriptionHandler = NULL;
//    _classObjectDestroyHandler = NULL;
//    _subclassHandler = NULL;
//}

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
    _isInternalCall = true;

    cn::vimfung::luascriptcore::LuaModule::onRegister(name, context);

    lua_State *state = context -> getLuaState();
    lua_getglobal(state, name.c_str());

    if (lua_istable(state, -1))
    {
        //关联本地类型
        lua_pushlightuserdata(state, this);
        lua_setfield(state, -2, "_nativeClass");

        //设置类型名称。since ver 1.3
        lua_pushstring(state, name.c_str());
        lua_setfield(state, -2, "name");

        //关联索引
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        //关联更新索引处理
        lua_pushlightuserdata(state, context);
        lua_pushcclosure(state, objectNewIndexHandler, 1);
        lua_setfield(state, -2, "__newindex");

        //创建方法
        lua_pushlightuserdata(state, context);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, objectCreateHandler, 2);
        lua_setfield(state, -2, "create");

        //子类化对象方法
        lua_pushlightuserdata(state, context);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, subClassHandler, 2);
        lua_setfield(state, -2, "subclass");

        //增加子类判断方法, since ver 1.3
        lua_pushlightuserdata(state, context);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, subclassOfHandler, 2);
        lua_setfield(state, -2, "subclassOf");

        if (_superClass != NULL)
        {
            //存在父类，则直接设置父类为元表
            lua_getglobal(state, _superClass -> getName().c_str());
            if (lua_istable(state, -1))
            {
                //设置父类元表
                lua_pushvalue(state, -1);
                lua_setmetatable(state, -3);

                //关联父类
                lua_setfield(state, -2, "super");
            }
        }
        else
        {
            //为根类，则创建一个table作为元表
            lua_newtable(state);

            //关联更新索引处理
            lua_pushlightuserdata(state, context);
            lua_pushcclosure(state, objectNewIndexHandler, 1);
            lua_setfield(state, -2, "__newindex");

            lua_setmetatable(state, -2);
        }

        //创建类实例元表
        luaL_newmetatable(state, name.c_str());

        lua_pushlightuserdata(state, this);
        lua_setfield(state, -2, "_nativeClass");

        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, objectDestroyHandler, 1);
        lua_setfield(state, -2, "__gc");

        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, objectToStringHandler, 1);
        lua_setfield(state, -2, "__tostring");

        if (_superClass != NULL)
        {
            //获取父级元表
            luaL_getmetatable(state, _superClass -> getName().c_str());
            if (lua_istable(state, -1))
            {
                //设置父类访问属性 since ver 1.3
                lua_pushvalue(state, -1);
                lua_setfield(state, -2, "super");

                //设置父类元表
                lua_setmetatable(state, -2);
            }
            else
            {
                lua_pop(state, 1);
            }
        }
        else
        {
            //Object类需要增加一些特殊方法
            //创建instanceOf方法 since ver 1.3
            lua_pushlightuserdata(state, context);
            lua_pushlightuserdata(state, this);
            lua_pushcclosure(state, instanceOfHandler, 2);
            lua_setfield(state, -2, "instanceOf");
        }
    }

    lua_pop(state, 1);

    _isInternalCall = false;
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
    _isInternalCall = true;
    char upperCStr[2] = {0};
    upperCStr[0] = toupper(fieldName[0]);
    std::string upperStr = upperCStr;
    std::string fieldNameStr = fieldName.c_str() + 1;
    std::string setterMethodName = "set" + upperStr + fieldNameStr;

    lua_State *state = getContext() -> getLuaState();
    luaL_getmetatable(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, setterMethodName.c_str());
        if (!lua_isnil(state, -1))
        {
            _isInternalCall = false;
            return;
        }
        lua_pop(state, 1);

        lua_getfield(state, -1, fieldName.c_str());
        if (!lua_isnil(state, -1))
        {
            _isInternalCall = false;
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

    _isInternalCall = false;
}

cn::vimfung::luascriptcore::modules::oo::LuaObjectClass* cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::getSupuerClass()
{
    return _superClass;
}

bool cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::subclassOf(LuaObjectClass *type)
{
    bool isSubclass = false;

    LuaObjectClass *tmpClass = this;
    while (tmpClass != NULL)
    {
        if (tmpClass -> getName() == type -> getName())
        {
            isSubclass = true;
            break;
        }

        tmpClass = tmpClass -> getSupuerClass();
    }

    return isSubclass;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::registerMethod(
        std::string methodName,
        LuaModuleMethodHandler handler)
{
    _isInternalCall = true;
    cn::vimfung::luascriptcore::LuaModule::registerMethod(methodName, handler);
    _isInternalCall = false;
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::registerInstanceMethod(
        std::string methodName,
        LuaInstanceMethodHandler handler)
{
    _isInternalCall = true;
    lua_State *state = getContext() -> getLuaState();
    luaL_getmetatable(state, getName().c_str());
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
    _isInternalCall = false;
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

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::createLuaInstance(LuaObjectInstanceDescriptor *objectDescriptor)
{
    lua_State *state = getContext() -> getLuaState();

    //先为实例对象在lua中创建内存
    LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
    //创建本地实例对象，赋予lua的内存块并进行保留引用
    ref -> value = objectDescriptor;

    //创建实例索引
    char buf[40];
#if _WINDOWS
    sprintf_s(buf, sizeof(buf), "Instance_%ld", InstanceSeed);
#else
    snprintf(buf, sizeof(buf), "Instance_%d", InstanceSeed);
#endif
    objectDescriptor -> setReferenceId(buf);
	InstanceSeed++;
	InstanceSeed %= INT_MAX;

    //对描述器进行引用
    objectDescriptor -> retain();

    //创建一个临时table作为元表，用于在lua上动态添加属性或方法
    lua_newtable(state);

    lua_pushvalue(state, -1);
    lua_setfield(state, -2, "__index");

    lua_pushcclosure(state, instanceNewIndexHandler, 0);
    lua_setfield(state, -2, "__newindex");

    lua_pushlightuserdata(state, this);
    lua_pushcclosure(state, objectDestroyHandler, 1);
    lua_setfield(state, -2, "__gc");

    lua_pushlightuserdata(state, this);
    lua_pushcclosure(state, objectToStringHandler, 1);
    lua_setfield(state, -2, "__tostring");

    lua_pushvalue(state, -1);
    lua_setmetatable(state, -3);

    luaL_getmetatable(state, getName().c_str());
    if (lua_istable(state, -1))
    {
        lua_setmetatable(state, -2);
    }
    else
    {
        lua_pop(state, 1);
    }

    lua_pop(state, 1);

    //将实例对象放入_instanceRefs_表中
    lua_getglobal(state, "_G");
    if (lua_istable(state, -1))
    {
        lua_getfield(state, -1, InstanceRefsTableName.c_str());
        if (lua_isnil(state, -1))
        {
            lua_pop(state, 1);

            //创建_instanceRefs_表，该表为弱引用表
            lua_newtable(state);

            //创建弱引用表元表
            lua_newtable(state);
            lua_pushstring(state, "kv");
            lua_setfield(state, -2, "__mode");
            lua_setmetatable(state, -2);

            lua_pushvalue(state, -1);
            lua_setfield(state, -3, InstanceRefsTableName.c_str());
        }

        //将实例对象放入表中
        lua_pushvalue(state, -3);
        lua_setfield(state, -2, objectDescriptor -> getReferenceId().c_str());

        lua_pop(state, 1);
    }
    lua_pop(state, 1);
}

void cn::vimfung::luascriptcore::modules::oo::LuaObjectClass::push(LuaObjectInstanceDescriptor *objectDescriptor)
{
    //对LuaObjectClass的类型对象需要进行特殊处理
    lua_State *state = getContext() -> getLuaState();

    bool hasExists = false;
    if (!objectDescriptor -> getReferenceId().empty())
    {
        //先查找_instanceRefs_中是否存在实例
        lua_getglobal(state, "_G");
        if (lua_istable(state, -1))
        {
            lua_getfield(state, -1, InstanceRefsTableName.c_str());
            if (lua_istable(state, -1))
            {
                lua_getfield(state, -1, objectDescriptor -> getReferenceId().c_str());
                if (lua_isuserdata(state, -1))
                {
                    //存在实例
                    lua_insert(state, -3);
                    hasExists = true;
                }
                else
                {
                    lua_pop(state, 1);
                }
            }

            lua_pop(state, 1);
        }

        lua_pop(state, 1);
    }

    if (!hasExists)
    {
        createLuaInstance(objectDescriptor);
    }
}
