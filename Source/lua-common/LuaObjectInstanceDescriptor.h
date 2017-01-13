//
// Created by 冯鸿杰 on 17/1/13.
//

#ifndef ANDROID_LUAOBJECTINSTANCEDESCRIPTOR_H
#define ANDROID_LUAOBJECTINSTANCEDESCRIPTOR_H

#include "LuaObjectDescriptor.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            namespace modules
            {
                namespace oo
                {
                    class LuaObjectClass;

                    /**
                     * 对象实例描述器
                     */
                    class LuaObjectInstanceDescriptor : public LuaObjectDescriptor
                    {

                    private:
                        //对象类型
                        LuaObjectClass *_objectClass;

                    public:

                        /**
                         * 初始化对象描述器
                         */
                        LuaObjectInstanceDescriptor ();

                        /**
                         * 初始化对象实例描述器
                         *
                         * @param instance 实例
                         * @param objectClass 对象类型
                         *
                         */
                        LuaObjectInstanceDescriptor(const void *instance, LuaObjectClass *objectClass);

                        /**
                        * 描述器对象析构方法
                        */
                        virtual ~LuaObjectInstanceDescriptor();

                    public:

                        /**
                         * 获取实例对象类型
                         *
                         * @return 对象类型
                         */
                        LuaObjectClass* getObjectClass();

                        /**
                         * 判断实例是否为指定类型
                         *
                         * @param objectClass 对象类型
                         *
                         * @return true 是， false 不是
                         */
                        virtual bool instanceOf (LuaObjectClass *objectClass);
                    };
                }
            }
        }
    }
}




#endif //ANDROID_LUAOBJECTINSTANCEDESCRIPTOR_H
