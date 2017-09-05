//
// Created by 冯鸿杰 on 16/9/27.
//

#include <limits>
#include <stdio.h>
#include <ctype.h>
#include "LuaObjectClass.h"
#include "LuaObjectInstanceDescriptor.h"
#include "LuaContext.h"
#include "LuaValue.h"
#include "LuaTuple.h"
#include "LuaSession.h"
#include "LuaEngineAdapter.hpp"

using namespace cn::vimfung::luascriptcore;
using namespace cn::vimfung::luascriptcore::modules::oo;

/**
 *  对象销毁处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectDestroyHandler (lua_State *state)
{
    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    if (LuaEngineAdapter::getTop(state) > 0 && LuaEngineAdapter::isUserdata(state, 1))
    {
        LuaArgumentList args;
        session -> parseArguments(args);
        
        LuaObjectDescriptor *objDesc = args[0] -> toObject();

        if (objectClass -> getObjectDestroyHandler() != NULL)
        {
            objectClass -> getObjectDestroyHandler() (objDesc);
        }

        //调用实例对象的destroy方法
        LuaEngineAdapter::pushValue(state, 1);

        LuaEngineAdapter::getField(state, -1, "destroy");
        if (LuaEngineAdapter::isFunction(state, -1))
        {
            LuaEngineAdapter::pushValue(state, 1);
            LuaEngineAdapter::pCall(state, 1, 0, 0);
        }
        else
        {
            LuaEngineAdapter::pop(state, 1);
        }

        LuaEngineAdapter::pop(state, 1);
        
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *value = *it;
            value -> release();
        }
    }

    

    objectClass -> getContext() -> destorySession(session);

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
    std::string desc;

    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    if (LuaEngineAdapter::getTop(state) > 0)
    {
        int type = LuaEngineAdapter::type(state, 1);

        //由于加入了实例的super对象，因此需要根据不同类型进行不同输出。since ver 1.3
        switch (type)
        {
            case LUA_TUSERDATA:
            {
                LuaArgumentList args;
                session -> parseArguments(args);

                LuaObjectDescriptor *instance = args[0] -> toObject();

                if (objectClass -> getObjectDescriptionHandler() != NULL)
                {
                    desc = objectClass -> getObjectDescriptionHandler()(instance);
                }

                if (desc.empty())
                {
                    char strbuf[256] = {0};
                    int size = sprintf(strbuf, "[%s object]", objectClass -> getName().c_str());
                    strbuf[size] = '\0';
                    
                    LuaEngineAdapter::pushString(state, strbuf);
                }
                else
                {
                    LuaEngineAdapter::pushString(state, desc.c_str());
                }
                break;
            }
            case LUA_TTABLE:
            {
                LuaEngineAdapter::pushString(state, "<SuperClass Type>");
                break;
            }
            default:
            {
                LuaEngineAdapter::pushString(state, "<Unknown Type>");
                break;
            }
        }
    }
    else
    {
        LuaEngineAdapter::pushString(state, "<Unknown Type>");
    }

    objectClass -> getContext() -> destorySession(session);

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
    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(2));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    if (objectClass -> getObjectCreatedHandler() != NULL)
    {
        objectClass -> getObjectCreatedHandler() (objectClass);
    }

    //调用实例对象的init方法
    LuaEngineAdapter::getField(state, -1, "init");
    if (LuaEngineAdapter::isFunction(state, -1))
    {
        LuaEngineAdapter::pushValue(state, -2);

        //将create传入的参数传递给init方法
        //-3 代表有3个非参数值在栈中，由栈顶开始计算，分别是：实例对象，init方法，实例对象
        int paramCount = LuaEngineAdapter::getTop(state) - 3;
        for (int i = 1; i <= paramCount; i++)
        {
            LuaEngineAdapter::pushValue(state, 1);
        }

        LuaEngineAdapter::pCall(state, paramCount + 1, 0, 0);
    }
    else
    {
        LuaEngineAdapter::pop(state, 1);
    }

    objectClass -> getContext() -> destorySession(session);

    return 1;
}

/**
 实例对象更新索引处理

 @param state 状态机
 @return 参数数量
 */
static int instanceNewIndexHandler (lua_State *state)
{
    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    //先找到实例对象的元表，向元表添加属性
    LuaEngineAdapter::getMetatable(state, 1);
    if (LuaEngineAdapter::isTable(state, -1))
    {
        LuaEngineAdapter::pushValue(state, 2);
        LuaEngineAdapter::pushValue(state, 3);
        LuaEngineAdapter::rawSet(state, -3);
    }

    objectClass -> getContext() -> destorySession(session);

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
    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(2));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    if (LuaEngineAdapter::getTop(state) > 0)
    {
        const char *subclassName = LuaEngineAdapter::checkString(state, 1);

        if (objectClass -> getSubClassHandler() != NULL)
        {
            objectClass -> getSubClassHandler()(objectClass, subclassName);
        }
    }

    objectClass -> getContext() -> destorySession(session);

    return 0;
}

/**
 判断是否是该类型的子类

 @param state 状态机
 @return 参数数量
 */
static int subclassOfHandler (lua_State *state)
{
    if (LuaEngineAdapter::getTop(state) == 0)
    {
        LuaEngineAdapter::pushBoolean(state, false);
        return 1;
    }

    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(2));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    if (LuaEngineAdapter::type(state, 1) == LUA_TTABLE)
    {
        LuaEngineAdapter::getField(state, 1, "_nativeClass");
        if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
        {
            LuaObjectClass *checkClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, -1);

            bool flag = objectClass -> subclassOf(checkClass);
            LuaEngineAdapter::pushBoolean(state, flag);
        }
        else
        {
            LuaEngineAdapter::pushBoolean(state, false);
        }
    }
    else
    {
        LuaEngineAdapter::pushBoolean(state, false);
    }

    objectClass -> getContext() -> destorySession(session);

    return 1;
}

/**
 判断是否是该类型的实例对象

 @param state 状态机
 @return 参数数量
 */
static int instanceOfHandler (lua_State *state)
{
    if (LuaEngineAdapter::getTop(state) < 2)
    {
        LuaEngineAdapter::pushBoolean(state, false);
        return 1;
    }

    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(2));
    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    //表示有实例对象传入
    LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::toUserdata(state, 1);
    LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)ref -> value;

    if (LuaEngineAdapter::type(state, 2) == LUA_TTABLE)
    {
        LuaEngineAdapter::getField(state, 2, "_nativeClass");
        if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
        {
            LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toPointer(state, -1);

            bool flag = objDesc -> instanceOf(objectClass);
            LuaEngineAdapter::pushBoolean(state, flag);
        }
        else
        {
            LuaEngineAdapter::pushBoolean(state, false);
        }
    }
    else
    {
        LuaEngineAdapter::pushBoolean(state, false);
    }

    objectClass -> getContext() -> destorySession(session);

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
    int returnCount = 0;

    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    std::string methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(2));

    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + methodName + " method error : missing self parameter, please call by instance:methodName(param)";
        objectClass -> getContext() -> raiseException(errMsg);

        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    LuaInstanceMethodHandler handler = objectClass -> getInstanceMethodHandler(methodName);
    if (handler != NULL)
    {
        LuaArgumentList args;
        session -> parseArguments(args);
        LuaObjectDescriptor *instance = args[0] -> toObject();

        LuaArgumentList methodArgs;
        for (int i = 1; i < args.size(); i++)
        {
            LuaValue *value = args[i];
            methodArgs.push_back(value);
        }

        cn::vimfung::luascriptcore::LuaValue *retValue = handler (instance, objectClass, methodName, methodArgs);

        if (retValue != NULL)
        {
            returnCount = session -> setReturnValue(retValue);
            //释放返回值
            retValue -> release();
        }

        //释放参数内存
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    objectClass -> getContext() -> destorySession(session);

    return returnCount;
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
    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    std::string fieldName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(2));

    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        objectClass -> getContext() -> raiseException(errMsg);

        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    LuaInstanceSetterHandler handler = objectClass -> getInstanceSetterHandler(fieldName);
    if (handler != NULL)
    {
        LuaArgumentList args;
        session -> parseArguments(args);
        LuaObjectDescriptor *instance = args[0] -> toObject();

        LuaValue *value = NULL;
        if (args.size() > 1)
        {
            value = args[1];
        }
        else
        {
            value = LuaValue::NilValue();
        }

        handler (instance, objectClass, fieldName, value);

        //释放参数内存
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    objectClass -> getContext() -> destorySession(session);

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
    LuaObjectClass *objectClass = (LuaObjectClass *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    std::string fieldName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(2));

    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        objectClass -> getContext() -> raiseException(errMsg);

        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaSession *session = objectClass -> getContext() -> makeSession(state);

    LuaInstanceGetterHandler handler = objectClass -> getGetterHandler(fieldName);
    if (handler != NULL)
    {
        LuaArgumentList args;
        session -> parseArguments(args);
        LuaObjectDescriptor *instance = args[0] -> toObject();

        LuaValue *retValue = handler (instance, objectClass, fieldName);

        session -> setReturnValue(retValue);

        if (retValue != NULL)
        {
            //释放返回值
            retValue -> release();
        }

        //释放参数内存
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    objectClass -> getContext() -> destorySession(session);

    return 1;
}

LuaObjectClass::LuaObjectClass(LuaObjectClass *superClass)
{
    _superClass = superClass;
    _classObjectCreatedHandler = NULL;
    _classObjectDescriptionHandler = NULL;
    _classObjectDestroyHandler = NULL;
    _subclassHandler = NULL;
}

void LuaObjectClass::onObjectCreated(LuaClassObjectCreatedHandler handler)
{
    _classObjectCreatedHandler = handler;
}

void LuaObjectClass::onObjectDestroy(LuaClassObjectDestroyHandler handler)
{
    _classObjectDestroyHandler = handler;
}

void LuaObjectClass::onObjectGetDescription (LuaClassObjectGetDescriptionHandler handler)
{
    _classObjectDescriptionHandler = handler;
}

void LuaObjectClass::onSubClass (LuaSubClassHandler handler)
{
    _subclassHandler = handler;
}

void LuaObjectClass::onRegister(const std::string &name, LuaContext *context)
{

    LuaModule::onRegister(name, context);

    lua_State *state = context -> getMainSession() -> getState();
    LuaEngineAdapter::getGlobal(state, name.c_str());

    if (LuaEngineAdapter::isTable(state, -1))
    {
        //关联本地类型
        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::setField(state, -2, "_nativeClass");

        //设置类型名称。since ver 1.3
        LuaEngineAdapter::pushString(state, name.c_str());
        LuaEngineAdapter::setField(state, -2, "name");

        //关联索引
        LuaEngineAdapter::pushValue(state, -1);
        LuaEngineAdapter::setField(state, -2, "__index");

        //创建方法
        LuaEngineAdapter::pushLightUserdata(state, context);
        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushCClosure(state, objectCreateHandler, 2);
        LuaEngineAdapter::setField(state, -2, "create");

        //子类化对象方法
        LuaEngineAdapter::pushLightUserdata(state, context);
        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushCClosure(state, subClassHandler, 2);
        LuaEngineAdapter::setField(state, -2, "subclass");

        //增加子类判断方法, since ver 1.3
        LuaEngineAdapter::pushLightUserdata(state, context);
        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushCClosure(state, subclassOfHandler, 2);
        LuaEngineAdapter::setField(state, -2, "subclassOf");

        if (_superClass != NULL)
        {
            //存在父类，则直接设置父类为元表
            std::string superClassName = _superClass -> getName();
            LuaEngineAdapter::getGlobal(state, superClassName.c_str());
            if (LuaEngineAdapter::isTable(state, -1))
            {
                //关联父类
                LuaEngineAdapter::pushValue(state, -1);
                LuaEngineAdapter::setField(state, -3, "super");

                //设置父类元表
                LuaEngineAdapter::setMetatable(state, -2);
            }
            else
            {
                LuaEngineAdapter::pop(state, 1);
            }
        }

        //创建类实例元表
        std::string metaName = _getMetaClassName(name);
        LuaEngineAdapter::newMetatable(state, metaName.c_str());

        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::setField(state, -2, "_nativeClass");

        LuaEngineAdapter::pushValue(state, -1);
        LuaEngineAdapter::setField(state, -2, "__index");

        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushCClosure(state, objectDestroyHandler, 1);
        LuaEngineAdapter::setField(state, 2, "__gc");

        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushCClosure(state, objectToStringHandler, 1);
        LuaEngineAdapter::setField(state, -2, "__tostring");

        //给类元表绑定该实例元表
        LuaEngineAdapter::getGlobal(state, name.c_str());
        LuaEngineAdapter::pushValue(state, -2);
        LuaEngineAdapter::setField(state, -2, "prototype");
        LuaEngineAdapter::pop(state, 1);

        if (_superClass != NULL)
        {
            //获取父级元表
            std::string superClassMetaName = _getMetaClassName(_superClass -> getName());
            LuaEngineAdapter::getMetatable(state, superClassMetaName.c_str());
            if (LuaEngineAdapter::isTable(state, -1))
            {
                //设置父类访问属性 since ver 1.3
                LuaEngineAdapter::pushValue(state, -1);
                LuaEngineAdapter::setField(state, -3, "super");

                //设置父类元表
                LuaEngineAdapter::setMetatable(state, -2);
            }
            else
            {
                LuaEngineAdapter::pop(state, 1);
            }
        }
        else
        {
            //Object类需要增加一些特殊方法
            //创建instanceOf方法 since ver 1.3
            LuaEngineAdapter::pushLightUserdata(state, context);
            LuaEngineAdapter::pushLightUserdata(state, this);
            LuaEngineAdapter::pushCClosure(state, instanceOfHandler, 2);
            LuaEngineAdapter::setField(state, -2, "instanceOf");
        }
    }

    LuaEngineAdapter::pop(state, 1);
}

LuaClassObjectCreatedHandler LuaObjectClass::getObjectCreatedHandler()
{
    return _classObjectCreatedHandler;
}

LuaClassObjectDestroyHandler LuaObjectClass::getObjectDestroyHandler()
{
    return  _classObjectDestroyHandler;
}

LuaClassObjectGetDescriptionHandler  LuaObjectClass::getObjectDescriptionHandler()
{
    return  _classObjectDescriptionHandler;
}

LuaSubClassHandler LuaObjectClass::getSubClassHandler()
{
    return _subclassHandler;
}

void LuaObjectClass::registerInstanceField(std::string fieldName, LuaInstanceGetterHandler getterHandler, LuaInstanceSetterHandler setterHandler)
{
    //与iOS中的属性getter和setter方法保持一致, getter直接是属性名称,setter则需要将属性首字母大写并在前面加上set
    char upperCStr[2] = {0};
    upperCStr[0] = (char)toupper(fieldName[0]);
    std::string upperStr = upperCStr;
    std::string fieldNameStr = fieldName.c_str() + 1;
    std::string setterMethodName = "set" + upperStr + fieldNameStr;

    lua_State *state = getContext() -> getMainSession() -> getState();
    std::string metaClassName = _getMetaClassName(getName());
    LuaEngineAdapter::getMetatable(state, metaClassName.c_str());
    if (LuaEngineAdapter::isTable(state, -1))
    {
        //设置Setter方法
        if (setterHandler != NULL)
        {
            LuaEngineAdapter::pushLightUserdata(state, this);
            LuaEngineAdapter::pushString(state, fieldName.c_str());
            LuaEngineAdapter::pushCClosure(state, instanceSetterRouteHandler, 2);

            LuaEngineAdapter::setField(state, -2, setterMethodName.c_str());

            _instanceSetterMap[fieldName] = setterHandler;
        }

        //注册Getter方法
        if (getterHandler != NULL)
        {
            LuaEngineAdapter::pushLightUserdata(state, this);
            LuaEngineAdapter::pushString(state, fieldName.c_str());
            LuaEngineAdapter::pushCClosure(state, instanceGetterRouteHandler, 2);
            
            LuaEngineAdapter::setField(state, -2, fieldName.c_str());
            
            _instanceGetterMap[fieldName] = getterHandler;
        }
    }

    LuaEngineAdapter::pop(state, 1);
}

LuaObjectClass* LuaObjectClass::getSupuerClass()
{
    return _superClass;
}

bool LuaObjectClass::subclassOf(LuaObjectClass *type)
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

void LuaObjectClass::registerMethod(
        std::string methodName,
        LuaModuleMethodHandler handler)
{
    LuaModule::registerMethod(methodName, handler);
}

void LuaObjectClass::registerInstanceMethod(
        std::string methodName,
        LuaInstanceMethodHandler handler)
{
    lua_State *state = getContext() -> getMainSession() -> getState();
    std::string metaClassName = _getMetaClassName(getName());
    LuaEngineAdapter::getMetatable(state, metaClassName.c_str());
    if (LuaEngineAdapter::isTable(state, -1))
    {
        LuaEngineAdapter::pushLightUserdata(state, this);
        LuaEngineAdapter::pushString(state, methodName.c_str());
        LuaEngineAdapter::pushCClosure(state, instanceMethodRouteHandler, 2);
        LuaEngineAdapter::setField(state, -2, methodName.c_str());

        _instanceMethodMap[methodName] = handler;
    }

    LuaEngineAdapter::pop(state, 1);
}

LuaInstanceMethodHandler LuaObjectClass::getInstanceMethodHandler(std::string methodName)
{
    LuaInstanceMethodMap::iterator it =  _instanceMethodMap.find(methodName.c_str());
    if (it != _instanceMethodMap.end())
    {
        return it -> second;
    }

    return NULL;
}

LuaInstanceSetterHandler LuaObjectClass::getInstanceSetterHandler(std::string fieldName)
{
    LuaInstanceSetterMap::iterator it =  _instanceSetterMap.find(fieldName.c_str());
    if (it != _instanceSetterMap.end())
    {
        return it -> second;
    }

    return NULL;
}

LuaInstanceGetterHandler LuaObjectClass::getGetterHandler(std::string fieldName)
{
    LuaInstanceGetterMap::iterator it =  _instanceGetterMap.find(fieldName.c_str());
    if (it != _instanceGetterMap.end())
    {
        return it -> second;
    }

    return NULL;
}

void LuaObjectClass::createLuaInstance(LuaObjectInstanceDescriptor *objectDescriptor)
{
    lua_State *state = getContext() -> getCurrentSession() -> getState();

    //先为实例对象在lua中创建内存
    LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::newUserdata(state, sizeof(LuaUserdataRef));
    //创建本地实例对象，赋予lua的内存块并进行保留引用
    ref -> value = objectDescriptor;

    //对描述器进行引用
    objectDescriptor -> retain();

    //创建一个临时table作为元表，用于在lua上动态添加属性或方法
    LuaEngineAdapter::newTable(state);

    LuaEngineAdapter::pushValue(state, -1);
    LuaEngineAdapter::setField(state, -2, "__index");

    LuaEngineAdapter::pushLightUserdata(state, this);
    LuaEngineAdapter::pushCClosure(state, instanceNewIndexHandler, 1);
    LuaEngineAdapter::setField(state, -2, "__newindex");
    
    LuaEngineAdapter::pushLightUserdata(state, this);
    LuaEngineAdapter::pushCClosure(state, objectDestroyHandler, 1);
    LuaEngineAdapter::setField(state, -2, "__gc");

    LuaEngineAdapter::pushLightUserdata(state, this);
    LuaEngineAdapter::pushCClosure(state, objectToStringHandler, 1);
    LuaEngineAdapter::setField(state, -2, "__tostring");

    LuaEngineAdapter::pushValue(state, -1);
    LuaEngineAdapter::setMetatable(state, -3);

    std::string metaClassName = _getMetaClassName(getName());
    LuaEngineAdapter::getMetatable(state, metaClassName.c_str());
    if (LuaEngineAdapter::isTable(state, -1))
    {
        LuaEngineAdapter::setMetatable(state, -2);
    }
    else
    {
        LuaEngineAdapter::pop(state, 1);
    }

    LuaEngineAdapter::pop(state, 1);
}

void LuaObjectClass::push(LuaObjectInstanceDescriptor *objectDescriptor)
{
    //对LuaObjectClass的类型对象需要进行特殊处理
    lua_State *state = getContext() -> getCurrentSession() -> getState();

    createLuaInstance(objectDescriptor);

    //调用默认init
    LuaEngineAdapter::getField(state, -1, "init");
    if (LuaEngineAdapter::isFunction(state, -1))
    {
        LuaEngineAdapter::pushValue(state, -2);
        LuaEngineAdapter::pCall(state, 1, 0, 0);
    }
    else
    {
        LuaEngineAdapter::pop(state, 1);
    }
}

std::string LuaObjectClass::_getMetaClassName(std::string className)
{
    return "_" + className + "_META_";
}
