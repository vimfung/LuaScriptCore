//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAJAVAOBJECTDESCRIPTOR_H
#define ANDROID_LUAJAVAOBJECTDESCRIPTOR_H

#include "LuaObjectDescriptor.h"
#include <jni.h>

using namespace cn::vimfung::luascriptcore;

/**
 * Java对象描述器，用于封装从Java层传入到原生层中。
 * 该类型继承于LuaObjectDescriptor，可用于直接导出到lua中使用。
 */
class LuaJavaObjectDescriptor : public LuaObjectDescriptor
{
public:
    /**
     * 初始化描述器对象
     *
     * @param env JNI环境
     * @param object Java对象
     */
    LuaJavaObjectDescriptor(JNIEnv *env, jobject object);

    /**
     * 描述器对象析构方法
     */
    virtual ~LuaJavaObjectDescriptor();

public:

    /**
     * 入栈数据
     *
     * @param context 上下文对象
     */
    void push(LuaContext *context);
};


#endif //ANDROID_LUAJAVAOBJECTDESCRIPTOR_H
