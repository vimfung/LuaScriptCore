//
// Created by 冯鸿杰 on 17/1/13.
//

#ifndef ANDROID_LUAJAVAOBJECTINSTANCEDESCRIPTOR_H
#define ANDROID_LUAJAVAOBJECTINSTANCEDESCRIPTOR_H

#include "LuaObjectInstanceDescriptor.h"
#include <jni.h>

class LuaJavaObjectClass;
//class LuaContext;

/**
 * Java对象实例描述器
 */
class LuaJavaObjectInstanceDescriptor : public cn::vimfung::luascriptcore::modules::oo::LuaObjectInstanceDescriptor
{
public:
    /**
     * 初始化描述器对象
     *
     * @param env JNI环境
     * @param object Java对象
     * @param objectClass 对象类型
     */
    LuaJavaObjectInstanceDescriptor(JNIEnv *env, jobject object, LuaJavaObjectClass *objectClass);

    /**
     * 描述器对象析构方法
     */
    virtual ~LuaJavaObjectInstanceDescriptor();

public:

    /**
     * 入栈数据
     *
     * @param context 上下文对象
     */
    void push(cn::vimfung::luascriptcore::LuaContext *context);

    /**
     * 判断实例是否为指定类型
     *
     * @param objectClass 对象类型
     *
     * @return true 是， false 不是
     */
    virtual bool instanceOf (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass);
};


#endif //ANDROID_LUAJAVAOBJECTINSTANCEDESCRIPTOR_H
