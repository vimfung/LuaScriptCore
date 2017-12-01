//
// Created by 冯鸿杰 on 2017/11/16.
//

#ifndef ANDROID_LUAJAVAEXPORTMETHODDESCRIPTOR_H
#define ANDROID_LUAJAVAEXPORTMETHODDESCRIPTOR_H

#include <jni.h>
#include <string>
#include "LuaExportMethodDescriptor.hpp"
#include "LuaValue.h"
#include "LuaSession.h"

using namespace cn::vimfung::luascriptcore;

/**
 * Java方法类型
 */
enum LuaJavaMethodType
{
    LuaJavaMethodTypeUnknown = 0,    //未知
    LuaJavaMethodTypeStatic = 1,     //类方法
    LuaJavaMethodTypeInstance = 2,   //实例方法
};

/**
 * Java导出方法
 */
class LuaJavaExportMethodDescriptor : public LuaExportMethodDescriptor
{
public:

    /**
     * 初始化
     *
     * @param name 方法名称
     * @param methodSignature 签名
     * @param type 方法类型，是类方法、实例方法、还是属性
     */
    LuaJavaExportMethodDescriptor(std::string name, std::string methodSignature, LuaJavaMethodType type);

    /**
     调用方法

     @param session 会话
     @param arguments 参数列表
     @return 返回值
    */
    virtual LuaValue* invoke(LuaSession *session, LuaArgumentList arguments);

private:

    /**
     * 方法类型
     */
    LuaJavaMethodType _type;

private:

    /**
     * 调用类方法
     *
     * @param session 会话
     * @param arguments 参数列表
     *
     * @return 返回值
     */
    LuaValue* invokeClassMethod(LuaSession *session, LuaArgumentList arguments);

    /**
     * 调用实例方法
     *
     * @param session 会话
     * @param arguments 参数列表
     *
     * @return 返回值
     */
    LuaValue* invokeInstanceMethod(LuaSession *session, LuaArgumentList arguments);
};


#endif //ANDROID_LUAJAVAEXPORTMETHODDESCRIPTOR_H
