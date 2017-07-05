//
//  LuaUnityClassObject.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityObjectClass.hpp"
#include "LuaObjectInstanceDescriptor.h"
#include "LuaUnityEnv.hpp"
#include "LuaContext.h"

using namespace cn::vimfung::luascriptcore;

static void luaInstanceCreateHandler (LuaObjectClass *objectClass)
{
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objectClass;
    
    LuaInstanceCreateHandlerPtr methodHandler = unityObjectClass -> getInstanceCreateHandler();
    if (methodHandler != NULL)
    {
        long long instance = methodHandler (unityObjectClass -> objectId());
        LuaObjectInstanceDescriptor *objDesc = new LuaObjectInstanceDescriptor((void *)instance, unityObjectClass);
        
        //创建Lua实例
        unityObjectClass -> createLuaInstance(objDesc);

        objDesc -> release();
    }
}

static void luaInstanceDestoryHandler (LuaObjectDescriptor *instance)
{
    LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance;
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objDesc -> getObjectClass();
    
    LuaInstanceDestoryHandlerPtr methodHandler = unityObjectClass -> getInstanceDestroyHandler();
    if (methodHandler != NULL)
    {
        methodHandler ((long long)objDesc -> getObject());
    }
}

static std::string luaInstanceDescriptHandler (LuaObjectDescriptor *instance)
{
    std::string desc;
    
    LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance;
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objDesc -> getObjectClass();
    
    LuaInstanceDescriptionHandlerPtr methodHandler = unityObjectClass -> getInstanceDescriptionHandler();
    if (methodHandler != NULL)
    {
        desc = methodHandler ((long long)objDesc -> getObject());
    }
    
    return desc;
}

static void luaSubclassHandler (LuaObjectClass *objectClass, std::string subclassName)
{
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objectClass;
    
    //创建子类描述
    LuaUnityObjectClass *subclass = new LuaUnityObjectClass(unityObjectClass);
    subclass -> setClassMethodHandler(unityObjectClass -> getClassMethodHandler());
    subclass -> setInstanceMethodHandler(unityObjectClass -> getInstanceMethodHandler());
    subclass -> setInstanceDescriptionHandler(unityObjectClass -> getInstanceDescriptionHandler());
    subclass -> setFieldGetterHandler(unityObjectClass -> getFieldGetterHandler());
    subclass -> setFieldSetterHandler(unityObjectClass -> getFieldSetterHandler());
    subclass -> setInstanceCreateHandler(unityObjectClass -> getInstanceCreateHandler());
    subclass -> setInstanceDestroyHandler(unityObjectClass -> getInstanceDestroyHandler());
    
    objectClass -> getContext() -> registerModule((const std::string)subclassName, subclass);
    subclass -> release();
}

LuaUnityObjectClass::LuaUnityObjectClass(LuaObjectClass *superClass)
    : LuaObjectClass(superClass)
{
    _classMethodHandler = NULL;
    _instanceMethodHandler = NULL;
    _fieldSetterHandler = NULL;
    _fieldGetterHandler = NULL;
    
    this -> onObjectCreated(luaInstanceCreateHandler);
    this -> onObjectDestroy(luaInstanceDestoryHandler);
    this -> onObjectGetDescription(luaInstanceDescriptHandler);
    this -> onSubClass(luaSubclassHandler);
}

void LuaUnityObjectClass::createLuaInstance(LuaObjectInstanceDescriptor *objectDescriptor)
{
    LuaObjectClass::createLuaInstance(objectDescriptor);
    
    LuaUnityEnv::sharedInstance() -> setNativeObjectId(objectDescriptor -> getObject(),
                                                       objectDescriptor -> objectId(),
                                                       objectDescriptor -> getLinkId());
}


void LuaUnityObjectClass::setClassMethodHandler(LuaModuleMethodHandlerPtr handler)
{
    _classMethodHandler = handler;
}

LuaModuleMethodHandlerPtr LuaUnityObjectClass::getClassMethodHandler()
{
    return _classMethodHandler;
}

void LuaUnityObjectClass::setInstanceMethodHandler(LuaInstanceMethodHandlerPtr handler)
{
    _instanceMethodHandler = handler;
}

LuaInstanceMethodHandlerPtr LuaUnityObjectClass::getInstanceMethodHandler()
{
    return _instanceMethodHandler;
}

void LuaUnityObjectClass::setFieldGetterHandler(LuaInstanceFieldGetterHandlerPtr handler)
{
    _fieldGetterHandler = handler;
}

LuaInstanceFieldGetterHandlerPtr LuaUnityObjectClass::getFieldGetterHandler()
{
    return _fieldGetterHandler;
}

void LuaUnityObjectClass::setFieldSetterHandler(LuaInstanceFieldSetterHandlerPtr handler)
{
    _fieldSetterHandler = handler;
}

LuaInstanceFieldSetterHandlerPtr LuaUnityObjectClass::getFieldSetterHandler()
{
    return _fieldSetterHandler;
}

void LuaUnityObjectClass::setInstanceCreateHandler(LuaInstanceCreateHandlerPtr handler)
{
    _instanceCreatedHandler = handler;
}

LuaInstanceCreateHandlerPtr LuaUnityObjectClass::getInstanceCreateHandler()
{
    return _instanceCreatedHandler;
}

void LuaUnityObjectClass::setInstanceDestroyHandler(LuaInstanceDestoryHandlerPtr handler)
{
    _instanceDestoryHandler = handler;
}

LuaInstanceDestoryHandlerPtr LuaUnityObjectClass::getInstanceDestroyHandler()
{
    return _instanceDestoryHandler;
}

void LuaUnityObjectClass::setInstanceDescriptionHandler(LuaInstanceDescriptionHandlerPtr handler)
{
    _instanceDescriptionHandler = handler;
}

LuaInstanceDescriptionHandlerPtr LuaUnityObjectClass::getInstanceDescriptionHandler()
{
    return _instanceDescriptionHandler;
}
