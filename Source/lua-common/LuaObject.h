//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAOBJECT_H
#define SAMPLE_LUAOBJECT_H

#include <string>

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
                virtual std::string typeName();
                void retain ();
                void release ();
                
                
                /**
                 查找对象

                 @param objectId 对象标识
                 @return 对象
                 */
                static LuaObject* findObject(int objectId);
                
            public:
                
                /**
                 序列化对象
                 
                 @param encoder 编码器
                 */
                virtual void serialization (LuaObjectEncoder *encoder);
            };
            
        }
    }
}


#endif //SAMPLE_LUAOBJECT_H
