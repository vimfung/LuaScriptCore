//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAJAVAOBJECTDESCRIPTOR_H
#define ANDROID_LUAJAVAOBJECTDESCRIPTOR_H

#include "LuaObjectDescriptor.h"
#include <jni.h>

using namespace cn::vimfung::luascriptcore;

class LuaJavaObjectDescriptor : public LuaObjectDescriptor
{
public:
    LuaJavaObjectDescriptor(jobject object);
};


#endif //ANDROID_LUAJAVAOBJECTDESCRIPTOR_H
