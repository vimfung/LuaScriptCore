//
//  LuaUnityClassObject.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//
#ifndef LuaUnityClassObject_hpp
#define LuaUnityClassObject_hpp

#include <stdio.h>
#include "LuaObjectClass.h"
#include "LuaUnityDefined.h"

using namespace cn::vimfung::luascriptcore::modules::oo;

/**
 Unity的Lua类对象
 */
class LuaUnityObjectClass : public LuaObjectClass
{
private:
    
    LuaInstanceCreateHandlerPtr _instanceCreatedHandler;
    LuaInstanceDestoryHandlerPtr _instanceDestoryHandler;
    LuaInstanceDescriptionHandlerPtr _instanceDescriptionHandler;
    LuaModuleMethodHandlerPtr _classMethodHandler;
    LuaInstanceMethodHandlerPtr _instanceMethodHandler;
    LuaInstanceFieldGetterHandlerPtr _fieldGetterHandler;
    LuaInstanceFieldSetterHandlerPtr _fieldSetterHandler;
    
public:
    
    /**
     * 初始化Lua类描述对象
     *
     * @param superClass 父级类型
     */
    LuaUnityObjectClass(LuaObjectClass *superClass);
    
    /**
     * 创建Lua实例对象
     *
     * @param objectDescriptor 对象描述器
     */
    virtual void createLuaInstance(LuaObjectInstanceDescriptor *objectDescriptor);
    
public:
    
    /**
     设置类方法处理器

     @param handler 处理器
     */
    void setClassMethodHandler(LuaModuleMethodHandlerPtr handler);
    
    /**
     获取类方法处理器

     @return 处理器
     */
    LuaModuleMethodHandlerPtr getClassMethodHandler();
    
    /**
     设置实例方法处理器

     @param handler 处理器
     */
    void setInstanceMethodHandler(LuaInstanceMethodHandlerPtr handler);
    
    /**
     获取实例方法处理器

     @return 处理器
     */
    LuaInstanceMethodHandlerPtr getInstanceMethodHandler();
    
    /**
     设置字段获取器

     @param handler 处理器
     */
    void setFieldGetterHandler(LuaInstanceFieldGetterHandlerPtr handler);
    
    /**
     获取字段获取器

     @return 处理器
     */
    LuaInstanceFieldGetterHandlerPtr getFieldGetterHandler();
    
    /**
     设置字段设置器

     @param handler 处理器
     */
    void setFieldSetterHandler(LuaInstanceFieldSetterHandlerPtr handler);
    
    /**
     获取字段设置器

     @return 处理器
     */
    LuaInstanceFieldSetterHandlerPtr getFieldSetterHandler();
    
    /**
     设置实例创建处理器

     @param handler 处理器
     */
    void setInstanceCreateHandler(LuaInstanceCreateHandlerPtr handler);
    
    /**
     获取实例创建处理器

     @return 处理器
     */
    LuaInstanceCreateHandlerPtr getInstanceCreateHandler();

    /**
     设置实例销毁处理器

     @param handler 处理器
     */
    void setInstanceDestroyHandler(LuaInstanceDestoryHandlerPtr handler);
    
    /**
     获取实例销毁处理器

     @return 处理器
     */
    LuaInstanceDestoryHandlerPtr getInstanceDestroyHandler();
    
    /**
     设置实例描述处理器

     @param handler 处理器
     */
    void setInstanceDescriptionHandler(LuaInstanceDescriptionHandlerPtr handler);
    
    /**
     获取实例描述处理器

     @return 处理器
     */
    LuaInstanceDescriptionHandlerPtr getInstanceDescriptionHandler();
    
};

#endif /* LuaUnityClassObject_hpp */
