//
// Created by 冯鸿杰 on 16/10/31.
//

#include "LuaJavaObjectDescriptor.h"
#include "LuaJavaEnv.h"
#include "LuaDefine.h"
#include "LuaJavaType.h"
#include "LuaObjectClass.h"

LuaJavaObjectDescriptor::LuaJavaObjectDescriptor(jobject object)
{
    JNIEnv *env = LuaJavaEnv::getEnv();

    //添加引用
    setObject((const void *)env -> NewGlobalRef(object));

    LuaJavaEnv::resetEnv(env);
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
        void **ref = LuaJavaEnv::getAssociateInstanceRef(obj);
        if (ref != NULL)
        {
            LuaJavaObjectDescriptor *objDesc = (LuaJavaObjectDescriptor *)*ref;

            //先为实例对象在lua中创建内存
            LuaJavaObjectDescriptor **copyRef = (LuaJavaObjectDescriptor **) lua_newuserdata(state, sizeof(LuaJavaObjectDescriptor *));
            *copyRef = objDesc;

            cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *objectClass = (cn::vimfung::luascriptcore::modules::oo::LuaObjectClass *)objDesc -> getUserdata();

            if (objectClass != NULL)
            {
                //将其关联元表
                luaL_getmetatable(state, objectClass->getName().c_str());
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

    }

    LuaJavaEnv::resetEnv(env);

    if (!process)
    {
        //调用父类方法
        LuaObjectDescriptor::push(context);
    }

}