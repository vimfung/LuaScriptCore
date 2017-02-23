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

static void luaInstanceCreateHandler (LuaObjectClass *objectClass)
{
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objectClass;
    
    LuaInstanceCreateHandlerPtr methodHandler = unityObjectClass -> getInstanceCreateHandler();
    if (methodHandler != NULL)
    {
        void *instance = methodHandler (unityObjectClass -> objectId());
        LuaObjectInstanceDescriptor *objDesc = new LuaObjectInstanceDescriptor(instance, unityObjectClass);
        
        //创建Lua实例
        unityObjectClass -> createLuaInstance(objDesc);
        
        objDesc -> release();
    }
}

static void luaInstanceDestoryHandler (cn::vimfung::luascriptcore::LuaUserdataRef instance)
{
    LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance -> value;
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objDesc -> getObjectClass();
    
    LuaInstanceDestoryHandlerPtr methodHandler = unityObjectClass -> getInstanceDestroyHandler();
    if (methodHandler != NULL)
    {
        methodHandler (objDesc -> getObject());
    }
}

static std::string luaInstanceDescriptHandler (cn::vimfung::luascriptcore::LuaUserdataRef instance)
{
    std::string desc;
    
    LuaObjectInstanceDescriptor *objDesc = (LuaObjectInstanceDescriptor *)instance -> value;
    LuaUnityObjectClass *unityObjectClass = (LuaUnityObjectClass *)objDesc -> getObjectClass();
    
    LuaInstanceDescriptionHandlerPtr methodHandler = unityObjectClass -> getInstanceDescriptionHandler();
    if (methodHandler != NULL)
    {
        desc = methodHandler (objDesc -> getObject());
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
                                                       objectDescriptor -> getReferenceId());
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
