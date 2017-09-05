//
// Created by 冯鸿杰 on 17/3/22.
//

#include <ctype.h>
#include "LuaClassImport.h"
#include "LuaExportClassProxy.h"
#include "LuaObjectDescriptor.h"
#include "LuaContext.h"
#include "LuaDefined.h"
#include "LuaTuple.h"
#include "LuaSession.h"
#include "LuaEngineAdapter.hpp"

using namespace cn::vimfung::luascriptcore;
using namespace cn::vimfung::luascriptcore::modules::oo;

/**
 实例引用表名称
 */
static std::string ProxyTableName = "_import_classes_";

/**
 * 方法路由处理
 */
static int classMethodRouteHandler(lua_State *state)
{
    int returnCount = 0;

    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaClassImport *classImport = (LuaClassImport *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(3));
    std::string methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(4));

    LuaSession *session = context -> makeSession(state);

    LuaClassMethodInvokeHandler handler = classImport -> getClassMethodInvokeHandler();
    if (handler != NULL)
    {
        LuaArgumentList args;
        session -> parseArguments(args);

        LuaValue *retValue = handler (context, classImport, clsDesc, methodName, args);
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

    context -> destorySession(session);

    return returnCount;
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

    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaClassImport *classImport = (LuaClassImport *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(3));
    std::string methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(4));

    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + methodName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);

        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaSession *session = context -> makeSession(state);

    LuaInstanceMethodInvokeHandler handler = classImport -> getInstanceMethodInvokeHandler();
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

        LuaValue *retValue = handler (context, classImport, clsDesc, instance, methodName, methodArgs);

        if (retValue != NULL)
        {
            //释放返回值
            if (retValue -> getType() == LuaValueTypeTuple)
            {
                returnCount = (int)retValue -> toTuple() -> count();
            }
            else
            {
                returnCount = 1;
            }

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

    context -> destorySession(session);

    return returnCount;
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
    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaClassImport *classImport = (LuaClassImport *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(3));
    std::string fieldName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(4));
    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);

        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaSession *session = context -> makeSession(state);

    LuaInstanceFieldGetterInvokeHandler handler = classImport -> getInstanceFieldGetterInvokeHandler();
    if (handler != NULL)
    {
        LuaArgumentList args;
        session -> parseArguments(args);

        LuaObjectDescriptor *instance = args[0] -> toObject();
        LuaValue *retValue = handler (context, classImport, clsDesc, instance, fieldName);

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

    context -> destorySession(session);

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
    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaClassImport *classImport = (LuaClassImport *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(3));
    std::string fieldName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(4));

    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);

        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaSession *session = context -> makeSession(state);

    LuaInstanceFieldSetterInvokeHandler handler = classImport -> getInstanceFieldSetterInvokeHandler();
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

        handler (context, classImport, clsDesc, instance, fieldName, value);

        //释放参数内存
        value -> release();
    }

    //回收内存
    context -> destorySession(session);

    return 0;
}

/**
 *  对象销毁处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectDestroyHandler (lua_State *state)
{
    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaSession *session = context -> makeSession(state);

    if (LuaEngineAdapter::getTop(state) > 0 && LuaEngineAdapter::isUserdata(state, 1))
    {
        //如果为userdata类型，则进行释放
        LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::toUserdata(state, 1);
        LuaObjectDescriptor *objectDescriptor = (LuaObjectDescriptor *)ref -> value;

        //释放内存
        objectDescriptor -> release();
    }

    context -> destorySession(session);

    return 0;
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
    LuaClassImport *classImport = (LuaClassImport *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    LuaCreateInstanceHandler createInstanceHandler = classImport -> getCreateInstanceHandler();
    if (createInstanceHandler == NULL)
    {
        return 0;
    }

    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(3));
    std::string clsName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(4));

    LuaSession *session = context -> makeSession(state);
    LuaObjectDescriptor *instanceDesc = createInstanceHandler(context, classImport, clsDesc);

    int retCount = 0;

    if (instanceDesc != NULL)
    {
        //先为实例对象在lua中创建内存
        LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::newUserdata(state, sizeof(LuaUserdataRef));
        //创建本地实例对象，赋予lua的内存块并进行保留引用
        ref -> value = instanceDesc;
        instanceDesc -> retain();

        //获取实例代理类型
        LuaEngineAdapter::getGlobal(state, "_G");
        if (LuaEngineAdapter::type(state, -1) == LUA_TTABLE)
        {
            LuaEngineAdapter::getField(state, -1, ProxyTableName.c_str());
            if (LuaEngineAdapter::type(state, -1) != LUA_TNIL)
            {
                //查找是否存在此类型的代理
                std::string prototypeClsName = clsName + "_prototype";
                LuaEngineAdapter::getField(state, -1, prototypeClsName.c_str());
                if (LuaEngineAdapter::isTable(state, -1))
                {
                    LuaEngineAdapter::setMetatable(state, -4);
                }
                else
                {
                    LuaEngineAdapter::pop(state, 1);
                }
            }

            LuaEngineAdapter::pop(state, 1);

        }

        LuaEngineAdapter::pop(state, 1);

        instanceDesc -> release();
        retCount = 1;
    }

    context -> destorySession(session);

    return retCount;
}

/**
 * 导出类型
 *
 * @param name 类型名称
 * @param context 上下文对象
 * @param classImport 类型导入
 *
 * @return 返回值数量
 */
static int exportsClass(const std::string &name, LuaContext *context, LuaClassImport *classImport)
{
    LuaAllowExportsClassHandler allowExportsHandler = classImport -> getAllowExportsClassHandler();
    LuaExportClassHandler exportClassHandler = classImport -> getExportClassHandler();
    if (allowExportsHandler == NULL || exportClassHandler == NULL)
    {
        return 0;
    }

    if (allowExportsHandler(context, classImport, name))
    {
        LuaCheckObjectSubclassHandler checkObjectSubclassHandler = classImport -> getCheckObjectSubclassHandler();
        if (checkObjectSubclassHandler != NULL && checkObjectSubclassHandler(context, classImport, name))
        {
            //如果类型为LuaObjectClass的子类，则由checkObjectSubclassHandler进行导入处理。
            return 1;
        }

        LuaExportClassProxy *classProxy = exportClassHandler(context, classImport, name);
        if (classProxy == NULL)
        {
            //无导出类型
            return 0;
        }

        //先判断_G下是否存在对象代理表
        lua_State *state = context -> getCurrentSession() -> getState();

        LuaEngineAdapter::getGlobal(state, "_G");
        if (LuaEngineAdapter::type(state, -1) == LUA_TTABLE)
        {
            LuaEngineAdapter::getField(state, -1, ProxyTableName.c_str());
            if (LuaEngineAdapter::type(state, -1) == LUA_TNIL)
            {
                //弹出nil
                LuaEngineAdapter::pop(state, 1);
                
                //创建对象代理表
                LuaEngineAdapter::newTable(state);

                LuaEngineAdapter::pushValue(state, -1);
                LuaEngineAdapter::setField(state, -3, ProxyTableName.c_str());
            }

            //查找是否存在此类型的代理
            LuaEngineAdapter::getField(state, -1, name.c_str());
            if (LuaEngineAdapter::type(state, -1) == LUA_TNIL)
            {
                //弹出nil
                LuaEngineAdapter::pop(state, 1);

                //导出代理类型
                LuaObjectDescriptor *clsDesc = classProxy -> getExportClass();
                LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::newUserdata(state, sizeof(LuaUserdataRef));
                ref -> value = clsDesc;
                clsDesc -> retain();

                //建立代理类元表
                LuaEngineAdapter::newTable(state);

                LuaEngineAdapter::pushValue(state, -1);
                LuaEngineAdapter::setField(state, -2, "__index");

                LuaEngineAdapter::pushLightUserdata(state, (void *)context);
                LuaEngineAdapter::pushCClosure(state, objectDestroyHandler, 1);
                LuaEngineAdapter::setField(state, -2, "__gc");

                //添加创建对象方法
                LuaEngineAdapter::pushLightUserdata(state, (void *)context);
                LuaEngineAdapter::pushLightUserdata(state, classImport);
                LuaEngineAdapter::pushLightUserdata(state, (void *)clsDesc);
                LuaEngineAdapter::pushString(state, name.c_str());
                LuaEngineAdapter::pushCClosure(state, objectCreateHandler, 4);
                LuaEngineAdapter::setField(state, -2, "create");

                //导出类方法
                LuaExportNameList clsNameList = classProxy -> allExportClassMethods();
                for (LuaExportNameList::iterator it = clsNameList.begin(); it != clsNameList.end() ; ++it)
                {
                    std::string methodName = *it;
                    LuaEngineAdapter::getField(state, -1, methodName.c_str());
                    if (LuaEngineAdapter::isNil(state, -1))
                    {
                        //尚未注册
                        LuaEngineAdapter::pop(state, 1);

                        LuaEngineAdapter::pushLightUserdata(state, context);
                        LuaEngineAdapter::pushLightUserdata(state, classImport);
                        LuaEngineAdapter::pushLightUserdata(state, clsDesc);
                        LuaEngineAdapter::pushString(state, methodName.c_str());
                        LuaEngineAdapter::pushCClosure(state, classMethodRouteHandler, 4);
                        
                        LuaEngineAdapter::setField(state, -2, methodName.c_str());
                    }
                    else
                    {
                        LuaEngineAdapter::pop(state, 1);
                    }
                }

                //关联元表
                LuaEngineAdapter::setMetatable(state, -2);

                //---------创建实例对象元表---------------
                LuaEngineAdapter::newTable(state);
                
                LuaEngineAdapter::pushValue(state, -1);
                LuaEngineAdapter::setField(state, -2, "__index");

                LuaEngineAdapter::pushLightUserdata(state, (void *)context);
                LuaEngineAdapter::pushCClosure(state, objectDestroyHandler, 1);
                LuaEngineAdapter::setField(state, -2, "__gc");

                //导出实例方法
                LuaExportNameList instNameList = classProxy -> allExportInstanceMethods();
                for (LuaExportNameList::iterator it = instNameList.begin(); it != instNameList.end() ; ++it)
                {
                    std::string methodName = *it;
                    LuaEngineAdapter::getField(state, -1, methodName.c_str());
                    if (LuaEngineAdapter::isNil(state, -1))
                    {
                        //尚未注册
                        LuaEngineAdapter::pop(state, 1);

                        LuaEngineAdapter::pushLightUserdata(state, context);
                        LuaEngineAdapter::pushLightUserdata(state, classImport);
                        LuaEngineAdapter::pushLightUserdata(state, clsDesc);
                        LuaEngineAdapter::pushString(state, methodName.c_str());
                        LuaEngineAdapter::pushCClosure(state, instanceMethodRouteHandler, 4);

                        LuaEngineAdapter::setField(state, -2, methodName.c_str());
                    }
                    else
                    {
                        LuaEngineAdapter::pop(state, 1);
                    }
                }

                //导出属性获取器列表
                LuaExportNameList getterfieldNameList = classProxy -> allExportGetterFields();
                for (LuaExportNameList::iterator it = getterfieldNameList.begin(); it != getterfieldNameList.end() ; ++it)
                {
                    std::string fieldName = *it;
                    LuaEngineAdapter::getField(state, -1, fieldName.c_str());
                    if (LuaEngineAdapter::isNil(state, -1))
                    {
                        //尚未注册
                        LuaEngineAdapter::pop(state, 1);

                        LuaEngineAdapter::pushLightUserdata(state, context);
                        LuaEngineAdapter::pushLightUserdata(state, classImport);
                        LuaEngineAdapter::pushLightUserdata(state, clsDesc);
                        LuaEngineAdapter::pushString(state, fieldName.c_str());
                        LuaEngineAdapter::pushCClosure(state, instanceGetterRouteHandler, 4);

                        LuaEngineAdapter::setField(state, -2, fieldName.c_str());
                    }
                    else
                    {
                        LuaEngineAdapter::pop(state, 1);
                    }
                }

                //导出属性设置器列表
                LuaExportNameList setterfieldNameList = classProxy -> allExportSetterFields();
                for (LuaExportNameList::iterator it = getterfieldNameList.begin(); it != getterfieldNameList.end() ; ++it)
                {
                    std::string fieldName = *it;

                    char upperCStr[2] = {0};
                    upperCStr[0] = (char)toupper(fieldName[0]);
                    std::string upperStr = upperCStr;
                    std::string fieldNameStr = fieldName.c_str() + 1;
                    std::string setterMethodName = "set" + upperStr + fieldNameStr;

                    LuaEngineAdapter::getField(state, -1, setterMethodName.c_str());
                    if (LuaEngineAdapter::isNil(state, -1))
                    {
                        //尚未注册
                        LuaEngineAdapter::pop(state, 1);

                        LuaEngineAdapter::pushLightUserdata(state, context);
                        LuaEngineAdapter::pushLightUserdata(state, classImport);
                        LuaEngineAdapter::pushLightUserdata(state, clsDesc);
                        LuaEngineAdapter::pushString(state, fieldName.c_str());
                        LuaEngineAdapter::pushCClosure(state, instanceSetterRouteHandler, 4);

                        LuaEngineAdapter::setField(state, -2, setterMethodName.c_str());
                    }
                    else
                    {
                        LuaEngineAdapter::pop(state, 1);
                    }
                }


                std::string prototypeClsName = name + "_prototype";
                LuaEngineAdapter::setField(state, -3, prototypeClsName.c_str());

                LuaEngineAdapter::pushValue(state, -1);
                LuaEngineAdapter::setField(state, -3, name.c_str());
            }

            //移除代理表
            LuaEngineAdapter::remove(state, -2);
            //移除_G
            LuaEngineAdapter::remove(state, -2);

            classProxy -> release();

            return 1;
        }

        LuaEngineAdapter::pop(state, 1);

        classProxy -> release();

        return 0;
    }

    return 0;
}

/**
 *  对象更新索引处理
 *
 *  @param state 状态机
 *
 *  @return 返回值数量
 */
static int setupProxyHandler (lua_State *state)
{
    LuaContext *context = (LuaContext *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaClassImport *classImport = (LuaClassImport *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    LuaSession *session = context -> makeSession(state);

    int retCount = 0;
    int top = LuaEngineAdapter::getTop(state);
    if (top > 0)
    {
        LuaValue *clsName = LuaValue::ValueByIndex(context, 1);
        if (clsName -> getType() == LuaValueTypeString)
        {
            //导出类型
            retCount = exportsClass(clsName -> toString(), context, classImport);
        }
    }

    context -> destorySession(session);

    return retCount;
}

void LuaClassImport::onRegister(const std::string &name,
                                LuaContext *context)
{
    _context = context;
    
    //注册一个ObjectProxy的全局方法，用于导出原生类型
    lua_State *state = context -> getMainSession() -> getState();

    //关联更新索引处理
    LuaEngineAdapter::pushLightUserdata(state, context);
    LuaEngineAdapter::pushLightUserdata(state, this);
    LuaEngineAdapter::pushCClosure(state, setupProxyHandler, 2);
    LuaEngineAdapter::setGlobal(state, "ClassImport");
}

bool LuaClassImport::setLuaMetatable(LuaContext *context, const std::string &className, LuaObjectDescriptor *objectDescriptor)
{
    lua_State *state = context -> getCurrentSession() -> getState();

    //查找类型是否为导出对象
    //获取实例代理类型
    LuaEngineAdapter::getGlobal(state, "_G");
    if (LuaEngineAdapter::type(state, -1) == LUA_TTABLE)
    {
        LuaEngineAdapter::getField(state, -1, ProxyTableName.c_str());
        if (LuaEngineAdapter::type(state, -1) != LUA_TNIL)
        {
            //查找是否存在此类型的代理
            std::string prototypeClsName = className + "_prototype";
            LuaEngineAdapter::getField(state, -1, prototypeClsName.c_str());
            if (LuaEngineAdapter::isTable(state, -1))
            {
                LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::newUserdata(state, sizeof(LuaUserdataRef));
                //讲本地对象赋予lua的内存块并进行保留引用
                ref -> value = objectDescriptor;
                objectDescriptor -> retain();

                LuaEngineAdapter::pushValue(state, -2);
                LuaEngineAdapter::setMetatable(state, -2);

                LuaEngineAdapter::remove(state, -2); //移除代理实例元表
                LuaEngineAdapter::remove(state, -2);  //移除代理表
                LuaEngineAdapter::remove(state, -2);  //移除_G

                return true;
            }

            LuaEngineAdapter::pop(state, 1);
        }

        LuaEngineAdapter::pop(state, 1);
    }

    LuaEngineAdapter::pop(state, 1);

    return false;
}

void LuaClassImport::onCheckObjectSubclass(LuaCheckObjectSubclassHandler handler)
{
    _checkObjectSubclassHandler = handler;
}

void LuaClassImport::onAllowExportsClass(LuaAllowExportsClassHandler handler)
{
    _allowExcportsClassHandler = handler;
}

void LuaClassImport::onExportsClass(LuaExportClassHandler handler)
{
    _exportClassHandler = handler;
}

void LuaClassImport::onCreateInstance(LuaCreateInstanceHandler handler)
{
    _createInstanceHandler = handler;
}

void LuaClassImport::onClassMethodInvoke(LuaClassMethodInvokeHandler handler)
{
    _classMethodInvokeHandler = handler;
}

void LuaClassImport::onInstanceMethodInvoke(LuaInstanceMethodInvokeHandler handler)
{
    _instanceMethodInvokeHandler = handler;
}

void LuaClassImport::onInstanceFieldGetterInvoke(LuaInstanceFieldGetterInvokeHandler handler)
{
    _instanceFieldGetterInvokeHandler = handler;
}

void LuaClassImport::onInstanceFieldSetterInvoke(LuaInstanceFieldSetterInvokeHandler handler)
{
    _instanceFieldSetterInvokeHandler = handler;
}

LuaClassMethodInvokeHandler LuaClassImport::getClassMethodInvokeHandler()
{
    return _classMethodInvokeHandler;
}

LuaInstanceMethodInvokeHandler LuaClassImport::getInstanceMethodInvokeHandler()
{
    return _instanceMethodInvokeHandler;
}

LuaInstanceFieldGetterInvokeHandler LuaClassImport::getInstanceFieldGetterInvokeHandler()
{
    return _instanceFieldGetterInvokeHandler;
}

LuaInstanceFieldSetterInvokeHandler LuaClassImport::getInstanceFieldSetterInvokeHandler()
{
    return _instanceFieldSetterInvokeHandler;
}

LuaCreateInstanceHandler LuaClassImport::getCreateInstanceHandler()
{
    return _createInstanceHandler;
}

LuaCheckObjectSubclassHandler LuaClassImport::getCheckObjectSubclassHandler()
{
    return _checkObjectSubclassHandler;
}

LuaAllowExportsClassHandler LuaClassImport::getAllowExportsClassHandler()
{
    return _allowExcportsClassHandler;
}

LuaExportClassHandler LuaClassImport::getExportClassHandler()
{
    return _exportClassHandler;
}
