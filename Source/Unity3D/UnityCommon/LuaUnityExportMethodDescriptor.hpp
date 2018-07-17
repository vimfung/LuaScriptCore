//
//  LuaUnityExportMethodDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/28.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaUnityExportClassMethodDescriptor_hpp
#define LuaUnityExportClassMethodDescriptor_hpp

#include <stdio.h>
#include "LuaExportMethodDescriptor.hpp"
#include "LuaUnityDefined.h"
#include "LuaSession.h"

using namespace cn::vimfung::luascriptcore;


typedef enum : unsigned int
{
    LuaUnityExportMethodTypeUnknown = 0,    //未知
    LuaUnityExportMethodTypeClass = 1,      //类方法
    LuaUnityExportMethodTypeInstance = 2,   //实例方法
} LuaUnityExportMethodType;

/**
 Unity导出类方法
 */
class LuaUnityExportMethodDescriptor : public LuaExportMethodDescriptor
{
public:
    
    /**
     初始化
     
     @param name 方法名称
     @param methodSignature 方法签名
     @param handler  类方法处理器
     */
    LuaUnityExportMethodDescriptor(std::string const& name, std::string const& methodSignature, LuaModuleMethodHandlerPtr handler);
    
    /**
     初始化
     
     @param name 方法名称
     @param methodSignature 方法签名
     @param handler  实例方法处理器
     */
    LuaUnityExportMethodDescriptor(std::string const& name, std::string const& methodSignature, LuaInstanceMethodHandlerPtr handler);
    
public:
    
    /**
     调用方法
     
     @param session 会话
     @param arguments 参数列表
     @return 返回值
     */
    virtual LuaValue* invoke(LuaSession *session, LuaArgumentList arguments);
    
private:
    
    /**
     方法类型
     */
    LuaUnityExportMethodType _methodType;
    
    /**
     实例方法处理器
     */
    LuaModuleMethodHandlerPtr _classMethodHandler;
    
    /**
     实例方法处理器
     */
    LuaInstanceMethodHandlerPtr _instanceMethodHandler;
    
private:
    
    
    /**
     调用类方法

     @param session 会话
     @param arguments 参数列表
     @return 返回值
     */
    LuaValue* invokeClassMethod(LuaSession *session, LuaArgumentList arguments);
    
    /**
     调用实例方法

     @param session 会话
     @param arguments 参数列表
     @return 返回值
     */
    LuaValue* invokeInstanceMethod(LuaSession *session, LuaArgumentList arguments);
};

#endif /* LuaUnityExportClassMethodDescriptor_hpp */
