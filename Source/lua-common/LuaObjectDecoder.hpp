//
//  LuaObjectDecoder.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef LuaObjectDecoder_hpp
#define LuaObjectDecoder_hpp

#include <stdio.h>
#include "LuaObject.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;
            
            /**
             对象解码器
             */
            class LuaObjectDecoder : public LuaObject
            {
            private:
                const void *_buf;
                int _offset;
                LuaContext *_context;
            public:
                
                /**
                 创建对象解码器
                 
                 @param context 上下文对象
                 */
                LuaObjectDecoder(LuaContext *context, const void *buf);
                
                /**
                 销毁对象解码器
                 */
                virtual ~LuaObjectDecoder();
                
            public:
                
                /**
                 获取上下文对象

                 @return 上下文对象
                 */
                LuaContext* getContext();
                
            public:
                
                /**
                 读取一个字符
                 
                 @return 字符值
                 */
                char readByte();
                
                /**
                 读取一个16位整型
                 
                 @return 16位整型值
                 */
                short readInt16();
                
                /**
                 读取一个32位整型
                 
                 @return 32位整型值
                 */
                int readInt32();
                
                /**
                 读取一个64位整型
                 
                 @return 64位整型值
                 */
                long long readInt64();
                
                /**
                 读取一个双精度浮点型
                 
                 @return 双精度浮点型值
                 */
                double readDouble();
                
                /**
                 读取一个字符串
                 
                 @return 字符串
                 */
                const std::string readString();
                
                /**
                 读取缓存数据
                 
                 @param bytes 缓存数据
                 @param length 缓存大小
                 */
                void readBytes(void **bytes, int *length);
                
                /**
                 读取对象

                 @return 对象
                 */
                LuaObject* readObject();
            };
            
        }
    }
}

#endif /* LuaObjectDecoder_hpp */
