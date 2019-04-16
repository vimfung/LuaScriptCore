//
// Created by 冯鸿杰 on 2017/11/16.
//

#ifndef ANDROID_LUAJAVAEXPORTTYPEDESCRIPTOR_H
#define ANDROID_LUAJAVAEXPORTTYPEDESCRIPTOR_H

#include "LuaExportTypeDescriptor.hpp"
#include <jni.h>
#include <string>

using namespace cn::vimfung::luascriptcore;

/**
 * Java导出类型
 */
class LuaJavaExportTypeDescriptor : public LuaExportTypeDescriptor
{
public:

    /**
      初始化

      @param typeName 类型名称
      @param env JNI环境
      @param jType Java类型
      @param parentTypeDescriptor 父级类型
     */
    LuaJavaExportTypeDescriptor (std::string &typeName, JNIEnv *env, jclass jType, LuaExportTypeDescriptor *parentTypeDescriptor);

    /**
     * 释放对象
     */
    virtual ~LuaJavaExportTypeDescriptor();

    /**
     * 获取Java类型
     *
     * @return Java类型
     */
    jclass getJavaType();

public:

    /**
     创建实例

     @param session 会话
     @return 实例对象
     */
    virtual LuaObjectDescriptor* createInstance(LuaSession *session);

    /**
     销毁实例

     @param session 会话
     @param objectDescriptor 实例对象
     */
    virtual void destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor);

private:

    //Java类型
    jclass _jType;
};


#endif //ANDROID_LUAJAVAEXPORTTYPEDESCRIPTOR_H
