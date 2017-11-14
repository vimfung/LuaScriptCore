//
//  LuaExportsTypeManager.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/16.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaExportsTypeManager.hpp"
#include "LuaExportTypeDescriptor.hpp"
#include "lua.hpp"
#include "LuaContext.h"
#include "LuaSession.h"
#include "LuaValue.h"
#include "LuaEngineAdapter.hpp"
#include "LuaExportMethodDescriptor.hpp"
#include "LuaObjectDescriptor.h"
#include "StringUtils.h"

using namespace cn::vimfung::luascriptcore;

/**
 *  创建对象时处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectCreateHandler (lua_State *state)
{
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    
    LuaSession *session = manager -> context() -> makeSession(state);
    
    LuaObjectDescriptor *objectDescriptor = typeDescriptor -> createInstance(session);
    manager -> _initLuaObject(objectDescriptor);
    objectDescriptor -> release();
    
    manager -> context() -> destorySession(session);
    
    return 1;
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
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    
    LuaContext *context = manager -> context();
    
    if (LuaEngineAdapter::getTop(state) == 0)
    {
        context -> raiseException("Miss the subclass name parameter");
        return 0;
    }
    
    LuaSession *session = context -> makeSession(state);
    
    std::string subclassName = LuaEngineAdapter::checkString(state, 1);
    
    //构建子类型描述
    LuaExportTypeDescriptor *subTypeDescriptor = typeDescriptor -> createSubType(session, subclassName);
    manager -> exportsType(subTypeDescriptor, false);
    subTypeDescriptor -> release();
    
    context -> destorySession(session);
    
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
    
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    
    LuaContext *context = manager -> context();
    LuaSession *session = context -> makeSession(state);
    
    if (LuaEngineAdapter::type(state, 1) == LUA_TTABLE)
    {
        LuaEngineAdapter::getField(state, 1, "_nativeClass");
        if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
        {
            LuaExportTypeDescriptor *checkType = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, -1);
            
            bool flag = typeDescriptor -> subtypeOfType(checkType);
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
    
    context -> destorySession(session);
    
    return 1;
}

/**
 类型转换为字符串处理
 
 @param state 状态
 @return 参数数量
 */
static int classToStringHandler (lua_State *state)
{
    std::string desc;
    
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    
    LuaContext *context = manager -> context();
    LuaSession *session = context -> makeSession(state);
    
    LuaExportTypeDescriptor *curType = NULL;
    LuaEngineAdapter::getField(state, 1, "_nativeType");
    if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
    {
        curType = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, -1);
    }
    
    if (typeDescriptor != NULL)
    {
        std::string descStr = StringUtils::format("[%s type]", curType -> typeName().c_str());
        LuaEngineAdapter::pushString(state, descStr.c_str());
    }
    else
    {
        context -> raiseException("Can not describe unknown type.");
        LuaEngineAdapter::pushNil(state);
    }
    
    context -> destorySession(session);
    return 1;
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
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toUserdata(state, LuaEngineAdapter::upValueIndex(1));
    
    LuaContext *context = manager -> context();
    LuaSession *session = context -> makeSession(state);
    
    if (LuaEngineAdapter::getTop(state) > 0 && LuaEngineAdapter::isUserdata(state, 1))
    {
        LuaArgumentList args;
        session -> parseArguments(args);
        
        LuaObjectDescriptor *objDesc = args[0] -> toObject();
        objDesc -> getTypeDescriptor() -> destroyInstance(session, objDesc);
        
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
        
        //释放实例对象
        objDesc -> release();
        
        for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
        {
            LuaValue *value = *it;
            value -> release();
        }
    }
    
    context -> destorySession(session);
    
    return 0;
}

/**
 转换Prototype为字符串处理
 
 @param state 状态
 @return 参数数量
 */
static int prototypeToStringHandler (lua_State *state)
{
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));

    LuaContext *context = manager -> context();
    LuaSession *session = context -> makeSession(state);

    LuaExportTypeDescriptor *typeDescriptor = NULL;

    LuaEngineAdapter::getField(state, 1, "_nativeType");
    if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
    {
        typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, -1);
    }

    if (typeDescriptor != NULL)
    {
        std::string descStr = StringUtils::format("[%s prototype]", typeDescriptor -> typeName().c_str());
        LuaEngineAdapter::pushString(state, descStr.c_str());
    }
    else
    {
        context -> raiseException("Can not describe unknown prototype.");
        LuaEngineAdapter::pushNil(state);
    }

    context -> destorySession(session);
    
    return 1;
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
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    
    LuaSession *session = manager -> context() -> makeSession(state);
    
    LuaArgumentList args;
    session -> parseArguments(args);
    
    LuaObjectDescriptor *objectDescriptor = args[0] -> toObject();
    LuaExportTypeDescriptor *typeDescriptor = objectDescriptor -> getTypeDescriptor();
    
    if (typeDescriptor)
    {
        std::string descStr = StringUtils::format("[%s object]", typeDescriptor -> typeName().c_str());
        LuaEngineAdapter::pushString(state, descStr.c_str());
    }
    else
    {
        manager -> context() -> raiseException("Can not describe unknown object.");
        LuaEngineAdapter::pushNil(state);
    }
    
    manager -> context() -> destorySession(session);

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

    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));

    LuaContext *context = manager -> context();
    LuaSession *session = context -> makeSession(state);

    //获取实例类型
    LuaExportTypeDescriptor *typeDescriptor = NULL;
    LuaEngineAdapter::getField(state, 1, "_nativeType");
    if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
    {
        typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, -1);
    }
    LuaEngineAdapter::pop(state, 1);

    if (typeDescriptor != NULL)
    {
        if (LuaEngineAdapter::type(state, 2) == LUA_TTABLE)
        {
            LuaEngineAdapter::getField(state, 2, "_nativeType");
            if (LuaEngineAdapter::type(state, -1) == LUA_TLIGHTUSERDATA)
            {
                LuaExportTypeDescriptor *checkTypeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, -1);
                
                bool flag = typeDescriptor -> subtypeOfType(checkTypeDescriptor);
                LuaEngineAdapter::pushBoolean(state, flag);

                return 1;
            }
        }
    }

    LuaEngineAdapter::pushBoolean(state, false);

    context -> destorySession(session);
    
    return 1;
}

/**
 类方法路由处理器
 
 @param state 状态
 @return 返回参数数量
 */
static int classMethodRouteHandler(lua_State *state)
{
    int retCount = 0;
    
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    const char *methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(3));
    
    LuaContext *context = manager -> context();
    LuaSession *session = context -> makeSession(state);
    
    LuaArgumentList args;
    session -> parseArguments(args);
    
    LuaExportMethodDescriptor *methodDescriptor = typeDescriptor -> getClassMethod(methodName, args);
    if (methodDescriptor != NULL)
    {
        LuaValue *retValue = methodDescriptor -> invoke(session, args);
        if (retValue != NULL)
        {
            retCount = session -> setReturnValue(retValue);
            //释放返回值
            retValue -> release();
        }
    }
    
    //释放参数内存
    for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
    {
        LuaValue *item = *it;
        item -> release();
    }
    
    //销毁会话
    context -> destorySession(session);
    
    return retCount;
}

/**
 实例方法路由处理
 
 @param state 状态
 @return 参数个数
 */
static int instanceMethodRouteHandler(lua_State *state)
{
    int returnCount = 0;
    
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaExportTypeDescriptor *typeDescriptor = (LuaExportTypeDescriptor *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(2));
    std::string methodName = LuaEngineAdapter::toString(state, LuaEngineAdapter::upValueIndex(3));
    
    LuaContext *context = manager -> context();
    
    if (LuaEngineAdapter::type(state, 1) != LUA_TUSERDATA)
    {
        std::string errMsg = "call " + methodName + " method error : missing self parameter, please call by instance:methodName(param)";
        context -> raiseException(errMsg);
        
        //回收内存
        LuaEngineAdapter::GC(state, LUA_GCCOLLECT, 0);
        
        return 0;
    }
    
    LuaSession *session = context -> makeSession(state);
    LuaArgumentList args;
    session -> parseArguments(args);
    
    LuaExportMethodDescriptor *methodDescriptor = typeDescriptor -> getInstanceMethod(methodName, args);
    if (methodDescriptor != NULL)
    {
        LuaValue *retValue = methodDescriptor -> invoke(session, args);
       
        if (retValue != NULL)
        {
            returnCount = session -> setReturnValue(retValue);
            //释放返回值
            retValue -> release();
        }
    }
    
    //释放参数内存
    for (LuaArgumentList::iterator it = args.begin(); it != args.end() ; ++it)
    {
        LuaValue *item = *it;
        item -> release();
    }
    
    context -> destorySession(session);
    
    return returnCount;
}

/**
 实例对象更新索引处理
 
 @param state 状态机
 @return 参数数量
 */
static int instanceNewIndexHandler (lua_State *state)
{
    LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));
    LuaSession *session = manager -> context() -> makeSession(state);
    
    //先找到实例对象的元表，向元表添加属性
    LuaEngineAdapter::getMetatable(state, 1);
    if (LuaEngineAdapter::isTable(state, -1))
    {
        LuaEngineAdapter::pushValue(state, 2);
        LuaEngineAdapter::pushValue(state, 3);
        LuaEngineAdapter::rawSet(state, -3);
    }
    
    manager -> context() -> destorySession(session);

    return 0;
}

/**
 导入原生类型处理
 
 @param state 状态
 @return 参数数量
 */
static int importNativeTypeHandler(lua_State *state)
{
    if (LuaEngineAdapter::getTop(state) > 0)
    {
        LuaExportsTypeManager *manager = (LuaExportsTypeManager *)LuaEngineAdapter::toPointer(state, LuaEngineAdapter::upValueIndex(1));

        std::string typeName = LuaEngineAdapter::checkString(state, 1);
        LuaEngineAdapter::getGlobal(state, typeName.c_str());
        if (LuaEngineAdapter::isNil(state, -1))
        {
            LuaEngineAdapter::pop(state, 1);

            //导入类型
            LuaExportTypeDescriptor *typeDescriptor = manager -> getExportTypeDescriptor(typeName);
            if (typeDescriptor)
            {
                manager -> exportsType(typeDescriptor, false);
                //重新获取
                LuaEngineAdapter::getGlobal(state, typeName.c_str());
            }
            else
            {
                LuaEngineAdapter::pushNil(state);
            }
        }
    }
    else
    {
        LuaEngineAdapter::pushNil(state);
    }
    
    return 1;
}

LuaExportsTypeManager::LuaExportsTypeManager(LuaContext *context)
{
    _context = context;
    
    _setupExportType();
    _setupNativeTypeMethod();
}

LuaExportsTypeManager::~LuaExportsTypeManager()
{
    //释放导出类型
    std::map<std::string, LuaExportTypeDescriptor*>::iterator it;
    for (it = _exportTypes.begin(); it != _exportTypes.end(); it++)
    {
        it -> second -> release();
    }
}

LuaContext* LuaExportsTypeManager::context()
{
    return _context;
}

LuaExportTypeDescriptor* LuaExportsTypeManager::getExportTypeDescriptor(std::string name)
{
    std::map<std::string, LuaExportTypeDescriptor*>::iterator it = _exportTypes.find(name);
    if (it != _exportTypes.end())
    {
        return it -> second;
    }
    
    return NULL;
}

void LuaExportsTypeManager::exportsType(LuaExportTypeDescriptor *typeDescriptor, bool lazyImport)
{
    if (!lazyImport)
    {
        //立即倒入类型
        lua_State *state = _context -> getMainSession() -> getState();
        
        //检测是否已经存在
        LuaEngineAdapter::getGlobal(state, typeDescriptor -> typeName().c_str());
        if (!LuaEngineAdapter::isNil(state, -1))
        {
            LuaEngineAdapter::pop(state, 1);
            
            std::string msg = StringUtils::format("The '%@' type of the specified name already exists!", typeDescriptor -> typeName().c_str());
            _context -> raiseException(msg);
            return;
        }
        
        //判断父类是否为导出类型
        LuaExportTypeDescriptor *parentTypeDescriptor = typeDescriptor -> parentTypeDescriptor();
        if (parentTypeDescriptor == NULL && typeDescriptor -> typeName() != "Object")
        {
            parentTypeDescriptor = getExportTypeDescriptor("Object");
        }
        
        if (parentTypeDescriptor != NULL)
        {
            LuaEngineAdapter::getGlobal(state, parentTypeDescriptor -> typeName().c_str());
            if (LuaEngineAdapter::isNil(state, -1))
            {
                //如果父类还没有注册，则进行注册操作
                exportsType(parentTypeDescriptor, false);
            }
            LuaEngineAdapter::pop(state, 1);
        }
        
        _exportsType(state, typeDescriptor);
    }
    
    _markType(typeDescriptor);
}

void LuaExportsTypeManager::_exportsType(lua_State *state, LuaExportTypeDescriptor *typeDescriptor)
{
    //创建类模块
    LuaEngineAdapter::newTable(state);
    
    //设置类名, since ver 1.3
    LuaEngineAdapter::pushString(state, typeDescriptor -> typeName().c_str());
    LuaEngineAdapter::setField(state, -2, "name");
    
    //关联本地类型
    LuaEngineAdapter::pushLightUserdata(state, (void *)(typeDescriptor));
    LuaEngineAdapter::setField(state, -2, "_nativeType");
    
    //导出声明的类方法
    _exportsClassMethods(state, typeDescriptor);
    
    //添加创建对象方法
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
    LuaEngineAdapter::pushCClosure(state, objectCreateHandler, 2);
    LuaEngineAdapter::setField(state, -2, "create");
    
    //添加子类化对象方法
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
    LuaEngineAdapter::pushCClosure(state, subClassHandler, 2);
    LuaEngineAdapter::setField(state, -2, "subclass");
    
    //增加子类判断方法, since ver 1.3
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
    LuaEngineAdapter::pushCClosure(state, subclassOfHandler, 2);
    LuaEngineAdapter::setField(state, -2, "subclassOf");
    
    //关联索引
    LuaEngineAdapter::pushValue(state, -1);
    LuaEngineAdapter::setField(state, -2, "__index");
    
    //类型描述
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
    LuaEngineAdapter::pushCClosure(state, classToStringHandler, 2);
    LuaEngineAdapter::setField(state, -2, "__tostring");
    
    //获取父类型
    LuaExportTypeDescriptor *parentTypeDescriptor = typeDescriptor -> parentTypeDescriptor();

    //关联父类模块
    if (parentTypeDescriptor != NULL)
    {
        //存在父类，则直接设置父类为元表
        LuaEngineAdapter::getGlobal(state, parentTypeDescriptor -> typeName().c_str());
        if (LuaEngineAdapter::isTable(state, -1))
        {
            //设置父类指向
            LuaEngineAdapter::pushValue(state, -1);
            LuaEngineAdapter::setField(state, -3, "super");
            
            //关联元表
            LuaEngineAdapter::setMetatable(state, -2);
        }
        else
        {
            LuaEngineAdapter::pop(state, 1);
        }
    }
    else
    {
        //Object需要创建一个新table来作为元表，否则无法使用元方法，如：print(Object);
        LuaEngineAdapter::newTable(state);
        
        //类型描述
        LuaEngineAdapter::pushLightUserdata(state, (void *)this);
        LuaEngineAdapter::pushCClosure(state, classToStringHandler, 1);
        LuaEngineAdapter::setField(state, -2, "__tostring");
        
        LuaEngineAdapter::setMetatable(state, -2);
    }
    
    LuaEngineAdapter::setGlobal(state, typeDescriptor -> typeName().c_str());

    //---------创建实例对象原型表---------------
    LuaEngineAdapter::newMetatable(state, typeDescriptor -> prototypeTypeName().c_str());
    
    LuaEngineAdapter::getGlobal(state, typeDescriptor -> typeName().c_str());
    LuaEngineAdapter::setField(state, -2, "class");
    
    LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
    LuaEngineAdapter::setField(state, -2, "_nativeType");
    
    LuaEngineAdapter::pushValue(state, -1);
    LuaEngineAdapter::setField(state, -2, "__index");
    
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushCClosure(state, objectDestroyHandler, 1);
    LuaEngineAdapter::setField(state, -2, "__gc");
    
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
    LuaEngineAdapter::pushCClosure(state, prototypeToStringHandler, 2);
    LuaEngineAdapter::setField(state, -2, "__tostring");
    
    //给类元表绑定该实例元表
    LuaEngineAdapter::getGlobal(state, typeDescriptor -> typeName().c_str());
    LuaEngineAdapter::pushValue(state, -2);
    LuaEngineAdapter::setField(state, -2, "prototype");
    LuaEngineAdapter::pop(state, 1);
    
    //导出实例方法
    _exportsInstanceMethods(state, typeDescriptor);
    
    if (parentTypeDescriptor != NULL)
    {
        //关联父类
        LuaEngineAdapter::getMetatable(state, parentTypeDescriptor -> prototypeTypeName().c_str());
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
        //Object需要创建一个新table来作为元表，否则无法使用元方法，如：print(Object);
        LuaEngineAdapter::newTable(state);
        
        LuaEngineAdapter::pushLightUserdata(state, (void *)this);
        LuaEngineAdapter::pushCClosure(state, objectDestroyHandler, 1);
        LuaEngineAdapter::setField(state, -2, "__gc");
        
        LuaEngineAdapter::pushLightUserdata(state, (void *)this);
        LuaEngineAdapter::pushCClosure(state, prototypeToStringHandler, 1);
        LuaEngineAdapter::setField(state, -2, "__tostring");
        
        LuaEngineAdapter::setMetatable(state, -2);
        
        //Object类需要增加一些特殊方法
        //创建instanceOf方法 since ver 1.3
        LuaEngineAdapter::pushLightUserdata(state, (void *)this);
        LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
        LuaEngineAdapter::pushCClosure(state, instanceOfHandler, 2);
        LuaEngineAdapter::setField(state, -2, "instanceOf");
    }
    
    LuaEngineAdapter::pop(state, 1);
}

void LuaExportsTypeManager::_exportsClassMethods(lua_State *state, LuaExportTypeDescriptor *typeDescriptor)
{
    std::list<std::string> methodNameList = typeDescriptor -> classMethodNameList();
    for (std::list<std::string>::iterator it = methodNameList.begin(); it != methodNameList.end(); it++)
    {
        LuaEngineAdapter::getField(state, -1, (*it).c_str());
        if (!LuaEngineAdapter::isFunction(state, -1))
        {
            LuaEngineAdapter::pop(state, 1);
            
            LuaEngineAdapter::pushLightUserdata(state, (void *)this);
            LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
            LuaEngineAdapter::pushString(state, (*it).c_str());
            LuaEngineAdapter::pushCClosure(state, classMethodRouteHandler, 3);
            
            LuaEngineAdapter::setField(state, -2, (*it).c_str());
        }
        else
        {
            LuaEngineAdapter::pop(state, 1);
        }
    }
}

void LuaExportsTypeManager::_exportsInstanceMethods(lua_State *state, LuaExportTypeDescriptor *typeDescriptor)
{
    std::list<std::string> methodNameList = typeDescriptor -> instanceMethodNameList();
    for (std::list<std::string>::iterator it = methodNameList.begin(); it != methodNameList.end(); it++)
    {
        LuaEngineAdapter::getField(state, -1, (*it).c_str());
        if (!LuaEngineAdapter::isFunction(state, -1))
        {
            LuaEngineAdapter::pop(state, 1);
            
            LuaEngineAdapter::pushLightUserdata(state, (void *)this);
            LuaEngineAdapter::pushLightUserdata(state, (void *)typeDescriptor);
            LuaEngineAdapter::pushString(state, (*it).c_str());
            LuaEngineAdapter::pushCClosure(state, instanceMethodRouteHandler, 3);
            
            LuaEngineAdapter::setField(state, -2, (*it).c_str());
        }
        else
        {
            LuaEngineAdapter::pop(state, 1);
        }
    }
}

void LuaExportsTypeManager::_setupExportType()
{
    //建立一个基类Object的描述
    std::string objectTypeName = "Object";
    LuaExportTypeDescriptor *objTypeDescriptor = new LuaExportTypeDescriptor(objectTypeName, NULL);
    exportsType(objTypeDescriptor, false);
}

void LuaExportsTypeManager::_setupNativeTypeMethod()
{
    lua_State *state = _context -> getMainSession() -> getState();
    
    LuaEngineAdapter::pushLightUserdata(state, (void *)this);
    LuaEngineAdapter::pushCClosure(state, importNativeTypeHandler, 1);
    LuaEngineAdapter::setGlobal(state, "nativeType");
}

void LuaExportsTypeManager::_markType(LuaExportTypeDescriptor *typeDescriptor)
{
    //先查找是否存在
    std::map<std::string, LuaExportTypeDescriptor*>::iterator typeIt = _exportTypes.find(typeDescriptor -> typeName());
    if (typeIt == _exportTypes.end())
    {
        typeDescriptor -> retain();
        _exportTypes[typeDescriptor -> typeName()] = typeDescriptor;
    }
}

void LuaExportsTypeManager::createLuaObject(LuaObjectDescriptor *objectDescriptor)
{
    LuaExportTypeDescriptor *typeDescriptor = objectDescriptor -> getTypeDescriptor();
    if (typeDescriptor)
    {
        lua_State *state = _context -> getCurrentSession() -> getState();
        LuaEngineAdapter::getGlobal(state, typeDescriptor -> typeName().c_str());
        if (LuaEngineAdapter::isNil(state, -1))
        {
            //导入类型
            exportsType(typeDescriptor, false);
        }
        LuaEngineAdapter::pop(state, 1);

        _initLuaObject(objectDescriptor);
    }
}

void LuaExportsTypeManager::_initLuaObject(LuaObjectDescriptor *objectDescriptor)
{
    lua_State *state = _context -> getCurrentSession() -> getState();
    
    _bindLuaInstance(objectDescriptor);
    
    //通过_createLuaInstanceWithState方法后会创建实例并放入栈顶
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
            LuaEngineAdapter::pushValue(state, i);
        }
        
        LuaEngineAdapter::pCall(state, paramCount + 1, 0, 0);
    }
    else
    {
        LuaEngineAdapter::pop(state, 1);
    }
}

void LuaExportsTypeManager::_bindLuaInstance(LuaObjectDescriptor *objectDescriptor)
{
    lua_State *state = _context -> getCurrentSession() -> getState();
    
    if (objectDescriptor != NULL)
    {
        //先为实例对象在lua中创建内存
        LuaUserdataRef ref = (LuaUserdataRef)LuaEngineAdapter::newUserdata(state, sizeof(LuaUserdataRef));
        //创建本地实例对象，赋予lua的内存块并进行保留引用
        ref -> value = objectDescriptor;
        
        //引用对象
        objectDescriptor -> retain();
    }
    
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
    
    LuaEngineAdapter::getMetatable(state, objectDescriptor -> getTypeDescriptor() -> prototypeTypeName().c_str());
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
