//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAJAVAOBJECTDESCRIPTOR_H
#define ANDROID_LUAJAVAOBJECTDESCRIPTOR_H

#include "LuaObjectDescriptor.h"
#include <jni.h>

using namespace cn::vimfung::luascriptcore;

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaExportTypeDescriptor;
        }
    }
}



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
     * @param context 上下文对象
     * @param env JNI环境
     * @param object Java对象
     */
    LuaJavaObjectDescriptor(LuaContext *context, JNIEnv *env, jobject object);

    /**
     * 初始化描述器对象
     *
     * @param context 上下文对象
     * @param env JNI环境
     * @param object Java对象
     * @param typeDescriptor 类型描述
     */
    LuaJavaObjectDescriptor(LuaContext *context, JNIEnv *env, jobject object, LuaExportTypeDescriptor *typeDescriptor);

    /**
     * 描述器对象析构方法
     */
    virtual ~LuaJavaObjectDescriptor();

    /**
     * 获取Java对象
     *
     * @return Java对象
     */
    jobject getJavaObject();
};


#endif //ANDROID_LUAJAVAOBJECTDESCRIPTOR_H
