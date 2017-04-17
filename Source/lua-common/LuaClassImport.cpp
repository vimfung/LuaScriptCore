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

#include "lunity.h"

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

    LuaContext *context = (LuaContext *)lua_topointer(state, lua_upvalueindex(1));
    LuaClassImport *classImport = (LuaClassImport *)lua_topointer(state, lua_upvalueindex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)lua_touserdata(state, lua_upvalueindex(3));
    std::string methodName = lua_tostring(state, lua_upvalueindex(4));

    LuaClassMethodInvokeHandler handler = classImport -> getClassMethodInvokeHandler();
    if (handler != NULL)
    {
        int top = lua_gettop(state);
        LuaArgumentList args;
        for (int i = 1; i <= top; i++)
        {
            LuaValue *value = context -> getValueByIndex(i);
            args.push_back(value);
        }

        LuaValue *retValue = handler (context, classImport, clsDesc, methodName, args);
        if (retValue != NULL)
        {
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
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *item = *it;
            item -> release();
        }
    }

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

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
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;

    int returnCount = 0;

    LuaContext *context = (LuaContext *)lua_topointer(state, lua_upvalueindex(1));
    LuaClassImport *classImport = (LuaClassImport *)lua_topointer(state, lua_upvalueindex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)lua_touserdata(state, lua_upvalueindex(3));
    std::string methodName = lua_tostring(state, lua_upvalueindex(4));

    if (lua_type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + methodName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);

        //回收内存
        lua_gc(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaUserdataRef instance = (LuaUserdataRef)lua_touserdata(state, 1);
    LuaInstanceMethodInvokeHandler handler = classImport -> getInstanceMethodInvokeHandler();
    
    if (handler != NULL)
    {
        int top = lua_gettop(state);
        LuaArgumentList args;
        for (int i = 2; i <= top; i++)
        {
            LuaValue *value = context -> getValueByIndex(i);
            args.push_back(value);
        }

        LuaValue *retValue = handler (context, classImport, clsDesc, instance, methodName, args);

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

    //回收内存
    lua_gc(state, LUA_GCCOLLECT, 0);

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
    using namespace cn::vimfung::luascriptcore;
    using namespace cn::vimfung::luascriptcore::modules::oo;

    LuaContext *context = (LuaContext *)lua_topointer(state, lua_upvalueindex(1));
    LuaClassImport *classImport = (LuaClassImport *)lua_topointer(state, lua_upvalueindex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)lua_touserdata(state, lua_upvalueindex(3));
    std::string fieldName = lua_tostring(state, lua_upvalueindex(4));

    if (lua_type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);

        //回收内存
        lua_gc(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaUserdataRef instance = (LuaUserdataRef)lua_touserdata(state, 1);
    LuaInstanceFieldGetterInvokeHandler handler = classImport -> getInstanceFieldGetterInvokeHandler();
    if (handler != NULL)
    {
        LuaValue *retValue = handler (context, classImport, clsDesc, instance, fieldName);
        if (retValue != NULL)
        {
            retValue -> push(context);
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

    LuaContext *context = (LuaContext *)lua_topointer(state, lua_upvalueindex(1));
    LuaClassImport *classImport = (LuaClassImport *)lua_topointer(state, lua_upvalueindex(2));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)lua_touserdata(state, lua_upvalueindex(3));
    std::string fieldName = lua_tostring(state, lua_upvalueindex(4));

    if (lua_type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + fieldName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);

        //回收内存
        lua_gc(state, LUA_GCCOLLECT, 0);

        return 0;
    }

    LuaUserdataRef instance = (LuaUserdataRef)lua_touserdata(state, 1);
    LuaInstanceFieldSetterInvokeHandler handler = classImport -> getInstanceFieldSetterInvokeHandler();
    if (handler != NULL)
    {
        LuaValue *value = NULL;
        int top = lua_gettop(state);
        if (top > 1)
        {
            value = context -> getValueByIndex(2);
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
    lua_gc(state, LUA_GCCOLLECT, 0);

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
    if (lua_gettop(state) > 0 && lua_isuserdata(state, 1))
    {
        //如果为userdata类型，则进行释放
        LuaUserdataRef ref = (LuaUserdataRef)lua_touserdata(state, 1);
        LuaObjectDescriptor *objectDescriptor = (LuaObjectDescriptor *)ref -> value;

        //释放内存
        objectDescriptor -> release();
    }

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
    LuaClassImport *classImport = (LuaClassImport *)lua_topointer(state, lua_upvalueindex(2));
    LuaCreateInstanceHandler createInstanceHandler = classImport -> getCreateInstanceHandler();
    if (createInstanceHandler == NULL)
    {
        return 0;
    }

    LuaContext *context = (LuaContext *)lua_topointer(state, lua_upvalueindex(1));
    LuaObjectDescriptor *clsDesc = (LuaObjectDescriptor *)lua_topointer(state, lua_upvalueindex(3));
    
    std::string clsName = lua_tostring(state, lua_upvalueindex(4));
    LuaObjectDescriptor *instanceDesc = createInstanceHandler(context, classImport, clsDesc);

    if (instanceDesc == NULL)
    {
        return 0;
    }

    //先为实例对象在lua中创建内存
    LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
    //创建本地实例对象，赋予lua的内存块并进行保留引用
    ref -> value = instanceDesc;
    instanceDesc -> retain();

    //获取实例代理类型
    lua_getglobal(state, "_G");
    if (lua_type(state, -1) == LUA_TTABLE)
    {
        lua_getfield(state, -1, ProxyTableName.c_str());
        if (lua_type(state, -1) != LUA_TNIL)
        {
            //查找是否存在此类型的代理
            std::string prototypeClsName = clsName + "_prototype";
            lua_getfield(state, -1, prototypeClsName.c_str());
            if (lua_istable(state, -1))
            {
                lua_setmetatable(state, -4);
            }
            else
            {
                lua_pop(state, 1);
            }
        }

        lua_pop(state, 1);

    }

    lua_pop(state, 1);

    instanceDesc -> release();

    return 1;
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
        LuaExportClassProxy *classProxy = exportClassHandler(context, classImport, name);
        if (classProxy == NULL)
        {
            //无导出类型
            return 0;
        }

        //先判断_G下是否存在对象代理表
        lua_State *state = context -> getLuaState();

        lua_getglobal(state, "_G");
        if (lua_type(state, -1) == LUA_TTABLE)
        {
            lua_getfield(state, -1, ProxyTableName.c_str());
            if (lua_type(state, -1) == LUA_TNIL)
            {
                //弹出nil
                lua_pop(state, 1);

                //创建对象代理表
                lua_newtable(state);

                lua_pushvalue(state, -1);
                lua_setfield(state, -3, ProxyTableName.c_str());
            }

            //查找是否存在此类型的代理
            lua_getfield(state, -1, name.c_str());
            if (lua_type(state, -1) == LUA_TNIL)
            {
                //弹出nil
                lua_pop(state, 1);

                //导出代理类型
                LuaObjectDescriptor *clsDesc = classProxy -> getExportClass();
                LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
                ref -> value = clsDesc;
                clsDesc -> retain();

                //建立代理类元表
                lua_newtable(state);

                lua_pushvalue(state, -1);
                lua_setfield(state, -2, "__index");

                lua_pushcfunction(state, objectDestroyHandler);
                lua_setfield(state, -2, "__gc");

                //添加创建对象方法
                lua_pushlightuserdata(state, (void *)context);
                lua_pushlightuserdata(state, classImport);
                lua_pushlightuserdata(state, (void *)clsDesc);
                lua_pushstring(state, name.c_str());
                lua_pushcclosure(state, objectCreateHandler, 4);
                lua_setfield(state, -2, "create");

                //导出类方法
                LuaExportNameList clsNameList = classProxy -> allExportClassMethods();
                for (LuaExportNameList::iterator it = clsNameList.begin(); it != clsNameList.end() ; ++it)
                {
                    std::string methodName = *it;
                    lua_getfield(state, -1, methodName.c_str());
                    if (lua_isnil(state, -1))
                    {
                        //尚未注册
                        lua_pop(state, 1);

                        lua_pushlightuserdata(state, context);
                        lua_pushlightuserdata(state, classImport);
                        lua_pushlightuserdata(state, clsDesc);
                        lua_pushstring(state, methodName.c_str());
                        lua_pushcclosure(state, classMethodRouteHandler, 4);

                        lua_setfield(state, -2, methodName.c_str());
                    }
                    else
                    {
                        lua_pop(state, 1);
                    }
                }

                //关联元表
                lua_setmetatable(state, -2);

                //---------创建实例对象元表---------------
                lua_newtable(state);

                lua_pushvalue(state, -1);
                lua_setfield(state, -2, "__index");

                lua_pushcfunction(state, objectDestroyHandler);
                lua_setfield(state, -2, "__gc");

                //导出实例方法
                LuaExportNameList instNameList = classProxy -> allExportInstanceMethods();
                for (LuaExportNameList::iterator it = instNameList.begin(); it != instNameList.end() ; ++it)
                {
                    std::string methodName = *it;
                    lua_getfield(state, -1, methodName.c_str());
                    if (lua_isnil(state, -1))
                    {
                        //尚未注册
                        lua_pop(state, 1);

                        lua_pushlightuserdata(state, context);
                        lua_pushlightuserdata(state, classImport);
                        lua_pushlightuserdata(state, clsDesc);
                        lua_pushstring(state, methodName.c_str());
                        lua_pushcclosure(state, instanceMethodRouteHandler, 4);

                        lua_setfield(state, -2, methodName.c_str());
                    }
                    else
                    {
                        lua_pop(state, 1);
                    }
                }

                //导出属性获取器列表
                LuaExportNameList getterfieldNameList = classProxy -> allExportGetterFields();
                for (LuaExportNameList::iterator it = getterfieldNameList.begin(); it != getterfieldNameList.end() ; ++it)
                {
                    std::string fieldName = *it;
                    lua_getfield(state, -1, fieldName.c_str());
                    if (lua_isnil(state, -1))
                    {
                        //尚未注册
                        lua_pop(state, 1);

                        lua_pushlightuserdata(state, context);
                        lua_pushlightuserdata(state, classImport);
                        lua_pushlightuserdata(state, clsDesc);
                        lua_pushstring(state, fieldName.c_str());
                        lua_pushcclosure(state, instanceGetterRouteHandler, 4);

                        lua_setfield(state, -2, fieldName.c_str());
                    }
                    else
                    {
                        lua_pop(state, 1);
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

                    lua_getfield(state, -1, setterMethodName.c_str());
                    if (lua_isnil(state, -1))
                    {
                        //尚未注册
                        lua_pop(state, 1);

                        lua_pushlightuserdata(state, context);
                        lua_pushlightuserdata(state, classImport);
                        lua_pushlightuserdata(state, clsDesc);
                        lua_pushstring(state, fieldName.c_str());
                        lua_pushcclosure(state, instanceSetterRouteHandler, 4);

                        lua_setfield(state, -2, setterMethodName.c_str());
                    }
                    else
                    {
                        lua_pop(state, 1);
                    }
                }


                std::string prototypeClsName = name + "_prototype";
                lua_setfield(state, -3, prototypeClsName.c_str());

                lua_pushvalue(state, -1);
                lua_setfield(state, -3, name.c_str());
            }

            //移除代理表
            lua_remove(state, -2);
            //移除_G
            lua_remove(state, -2);

            classProxy -> release();

            return 1;
        }

        lua_pop(state, 1);

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
    LuaContext *context = (LuaContext *)lua_topointer(state, lua_upvalueindex(1));
    LuaClassImport *classImport = (LuaClassImport *)lua_topointer(state, lua_upvalueindex(2));

    int top = lua_gettop(state);
    if (top > 0)
    {
        LuaValue *clsName = context -> getValueByIndex(1);
        if (clsName -> getType() == LuaValueTypeString)
        {
            //导出类型
            return exportsClass(clsName -> toString(), context, classImport);
        }
    }

    return 0;
}

void LuaClassImport::onRegister(const std::string &name,
                                LuaContext *context)
{
    _context = context;
    
    //注册一个ObjectProxy的全局方法，用于导出原生类型
    lua_State *state = context -> getLuaState();

    //关联更新索引处理
    lua_pushlightuserdata(state, context);
    lua_pushlightuserdata(state, this);
    lua_pushcclosure(state, setupProxyHandler, 2);
    lua_setglobal(state, "ClassImport");
}

bool LuaClassImport::setLuaMetatable(LuaContext *context, const std::string &className, LuaObjectDescriptor *objectDescriptor)
{
    lua_State *state = context -> getLuaState();

    //查找类型是否为导出对象
    //获取实例代理类型
    lua_getglobal(state, "_G");
    if (lua_type(state, -1) == LUA_TTABLE)
    {
        lua_getfield(state, -1, ProxyTableName.c_str());
        if (lua_type(state, -1) != LUA_TNIL)
        {
            //查找是否存在此类型的代理
            std::string prototypeClsName = className + "_prototype";
            lua_getfield(state, -1, prototypeClsName.c_str());
            if (lua_istable(state, -1))
            {
                LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
                //讲本地对象赋予lua的内存块并进行保留引用
                ref -> value = objectDescriptor;
                objectDescriptor -> retain();

                lua_pushvalue(state, -2);
                lua_setmetatable(state, -2);

                lua_remove(state, -2);  //移除代理实例元表
                lua_remove(state, -2);  //移除代理表
                lua_remove(state, -2);  //移除_G

                return true;
            }

            lua_pop(state, 1);
        }

        lua_pop(state, 1);
    }

    lua_pop(state, 1);

    return false;
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

LuaAllowExportsClassHandler LuaClassImport::getAllowExportsClassHandler()
{
    return _allowExcportsClassHandler;
}

LuaExportClassHandler LuaClassImport::getExportClassHandler()
{
    return _exportClassHandler;
}
