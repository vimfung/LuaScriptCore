//
// Created by 冯鸿杰 on 16/10/31.
//

#ifndef ANDROID_LUAOBJECTDESC_H
#define ANDROID_LUAOBJECTDESC_H

#include "LuaManagedObject.h"
#include <string>
#include <map>

namespace cn {
    namespace vimfung {
        namespace luascriptcore {

            class LuaContext;
            class LuaObjectDecoder;
            class LuaObjectDescriptor;
            class LuaExportTypeDescriptor;
            
            /**
             * 用户数据
             */
            typedef std::map<std::string, std::string> LuaObjectDescriptorUserData;

            /**
             * 对象入栈数据过滤器
             */
            typedef bool (*LuaObjectDescriptorPushFilter) (LuaContext *context, LuaObjectDescriptor *objectDescriptor);

            /**
             * Lua对象描述器
             */
            class LuaObjectDescriptor : public LuaManagedObject
            {
            private:

                /**
                 * 对象
                 */
                void *_object;

				/**
				类型描述器
				*/
				LuaExportTypeDescriptor *_typeDescriptor;
                
                /**
                 * 用户自定义数据
                 */
                LuaObjectDescriptorUserData _userdata;

                /**
                 * 连接ID
                 */
                std::string _linkId;

            protected:

                /**
                 * 设置对象
                 *
                 * @param object 对象引用
                 */
                void setObject(const void *object);

            public:

                /**
                 * 添加对象入栈过滤器
                 *
                 * @param filter 过滤器
                 */
                static void addPushFilter(LuaObjectDescriptorPushFilter filter);

            public:

                /**
                 * 初始化对象描述器
                 */
                LuaObjectDescriptor();
                
                /**
                 * 初始化对象描述器
                 *
                 * @param object 对象指针
                 */
                LuaObjectDescriptor(const void *object);
                
                /**
                 初始化
                 
                 @param object 对象
                 @param typeDescriptor 类型描述器
                 */
                LuaObjectDescriptor(void *object, LuaExportTypeDescriptor *typeDescriptor);
                
                /**
                 * 初始化, 在反序列化对象时会触发该方法
                 *
                 * @param decoder 解码器
                 */
                LuaObjectDescriptor (LuaObjectDecoder *decoder);

                /**
                 * 释放对象描述器
                 */
                virtual ~LuaObjectDescriptor();
                
                /**
                 获取类型名称
                 
                 @return 类型名称
                 */
                virtual std::string typeName();
                
                /**
                 序列化对象
                 
                 @param encoder 编码器
                 */
                virtual void serialization (LuaObjectEncoder *encoder);
                
                
                /**
                 设置用户数据

                 @param key 键名称
                 @param value 键值
                 */
                void setUserdata(std::string key, std::string value);
                
                /**
                 获取用户数据

                 @param key 键名称
                 @return 键值
                 */
                std::string getUserdata(std::string key);

            public:

                /**
                 * 获取对象
                 *
                 * @return 对象引用
                 */
                const void* getObject();
                
                /**
                 获取类型,当该值为NULL时表示非导出类型。

                 @return 类型对象
                 */
                LuaExportTypeDescriptor* getTypeDescriptor();

            public:

                /**
                 * 获取对象标识
                 *
                 * @return 对象标识
                 */
                virtual std::string getLinkId();

                /**
                 * 入栈数据
                 *
                 * @param context 上下文对象
                 */
                virtual void push(LuaContext *context);
            };
        }
    }
}


#endif //ANDROID_LUAOBJECTDESC_H
