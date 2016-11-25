//
//  LuaNativeClass.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef LuaNativeClass_hpp
#define LuaNativeClass_hpp

#include <stdio.h>
#include <string>
#include <map>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            
            class LuaObjectDecoder;
            
            #define DECLARE_NATIVE_CLASS(class) \
                static void* create##class##Instance()\
                {\
                    return new class();\
                }\
                static void* create##class##InstanceByDecoder(LuaObjectDecoder *decoder)\
                {\
                    return new class(decoder);\
                }\
                static LuaNativeClass *__##class##ClassType = new LuaNativeClass(#class, create##class##Instance, create##class##InstanceByDecoder);\
            
            /**
             创建类实例处理器
             
             @return 实例对象
             */
            typedef void* (*CreateInstanceHandler)(void);
            
            /**
             创建类实例处理器
             
             @param decoder 对象解码器
             
             @return 实例对象
             */
            typedef void* (*CreateInstanceByDecoderHandler) (LuaObjectDecoder *decoder);
            
            /**
             原生类
             */
            class LuaNativeClass
            {
            private:
                std::string _className;
                CreateInstanceHandler _createInstanceHandler;
                CreateInstanceByDecoderHandler _createInstanceByDecoderHandler;

            public:
                LuaNativeClass(std::string className,
                               CreateInstanceHandler createInstanceHandler,
                               CreateInstanceByDecoderHandler createInstanceByDecoderHandler);
                
            public:
                
                /**
                 创建实例

                 @return 实例对象
                 */
                void* createInstance();
                
                
                /**
                 创建实例

                 @param decoder 对象解码器
                 @return 实例对象
                 */
                void* createInstance(LuaObjectDecoder *decoder);
                
            public:
                
                /**
                 注册类型
                 
                 @param className 类名称
                 @param nativeClass 类型
                 */
                static void registerClass(std::string className, LuaNativeClass* nativeClass);
                
                
                /**
                 查找类型

                 @param className 类名称
                 @return 类型
                 */
                static LuaNativeClass* findClass(std::string className);
            };
        }
    }
}

#endif /* LuaNativeClass_hpp */
