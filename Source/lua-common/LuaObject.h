//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAOBJECT_H
#define SAMPLE_LUAOBJECT_H

#include <string>
#include "LuaNativeClass.hpp"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaObjectEncoder;
            class LuaObjectDecoder;
            
            class LuaObject
            {
            private:
                int _retainCount;
                int _objectId;
                
            public:
                LuaObject ();
                LuaObject (LuaObjectDecoder *decoder);
                virtual ~LuaObject();

            public:
                int objectId ();
                void retain ();
                void release ();
                
            public:
                
                /**
                 设置映射类型，在不同平台上对应的对象类型可能有所不同，因此借助此参数在序列化对象返回给不同平台时附加该类型名称。

                 @param className 类型名称
                 @param mappingClassName 映射的类型名称
                 */
                static void setMappingClassType(std::string className, std::string mappingClassName);
                
                /**
                 序列化对象
                 
                 @param className 类型名称
                 @param encoder 编码器
                 */
                virtual void serialization (std::string className, LuaObjectEncoder *encoder);
            };
            
        }
    }
}


#endif //SAMPLE_LUAOBJECT_H
