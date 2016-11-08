//
// Created by 冯鸿杰 on 16/10/31.
//

#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaJavaType.h"
#include "LuaObjectClass.h"
#include "LuaPointer.h"
#include "../../../../../lua-core/src/lua.hpp"

LuaJavaObjectDescriptor::LuaJavaObjectDescriptor(JNIEnv *env, jobject object)
{
    //添加引用
    setObject((const void *)env -> NewGlobalRef(object));
}

LuaJavaObjectDescriptor::~LuaJavaObjectDescriptor()
{
    JNIEnv *env = LuaJavaEnv::getEnv();
    //移除引用
    env -> DeleteGlobalRef((jobject)getObject());
    LuaJavaEnv::resetEnv(env);
}

void LuaJavaObjectDescriptor::push(LuaContext *context)
{
    bool process = false;

    JNIEnv *env = LuaJavaEnv::getEnv();

    jobject obj = (jobject)getObject();
    if (env -> IsInstanceOf(obj, LuaJavaType::luaObjectClass(env)))
    {
        //对LuaObjectClass的类型对象需要进行特殊处理
        lua_State *state = context -> getLuaState();

        //获取关联引用
        if (_userdataRef == NULL)
        {
            //创建关联引用
            LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
            ref -> value = this;
            setReference(ref);
        }
        else
        {
            lua_pushlightuserdata(state, _userdataRef);
        }

        cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass = (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *)this -> getUserdata();

        std::string className;
        if (objectClass == NULL)
        {
            //如果Descriptor的userdata中没有类型信息，则表示该对象是由Java层创建，直接获取类型名称
            className = LuaJavaEnv::getJavaClassNameByInstance(env, obj);
        }
        else
        {
            className = objectClass->getName();
        }

        if (!className.empty())
        {
            //将其关联元表
            luaL_getmetatable(state, className.c_str());
            if (lua_istable(state, -1))
            {
                lua_setmetatable(state, -2);
            }
            else
            {
                lua_pop(state, 1);
            }
        }

        process = true;

    }

    LuaJavaEnv::resetEnv(env);

    if (!process)
    {
        //调用父类方法
        LuaObjectDescriptor::push(context);
    }

}