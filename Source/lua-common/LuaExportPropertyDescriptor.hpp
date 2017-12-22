//
//  LuaExportPropertyDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/30.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaExportPropertyDescriptor_hpp
#define LuaExportPropertyDescriptor_hpp

#include <stdio.h>
#include <string>
#include "LuaDefined.h"
#include "LuaObject.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaSession;
            class LuaFunction;
            class LuaExportTypeDescriptor;
            class LuaObjectDescriptor;
            
            /**
             属性描述
             */
            class LuaExportPropertyDescriptor : public LuaObject
            {
            public:
                
                /**
                 初始化
                 
                 @param name 属性名称
                 @param canRead 是否能读
                 @param canWrite 是否能写
                 */
                LuaExportPropertyDescriptor(std::string name, bool canRead, bool canWrite);
                
                /**
                 初始化
                 
                 @param name 属性名称
                 @param getter 获取处理器
                 @param setter 设置处理器
                 */
                LuaExportPropertyDescriptor(std::string name, LuaFunction *getter, LuaFunction *setter);
                
                /**
                 销毁对象
                 */
                ~LuaExportPropertyDescriptor();
                
                /**
                 是否能读
                 
                 @return true 可读，false 不可读
                 */
                bool canRead();
                
                
                /**
                 是否能写

                 @return true 可写，false 不可写
                 */
                bool canWrite();
                
                /**
                 获取属性名称

                 @return 属性名称
                 */
                std::string name();
                
                /**
                 调用Getter方法
                 
                 @param session 会话
                 @param instance 实例对象
                 @return 返回值
                 */
                virtual LuaValue* invokeGetter(LuaSession *session, LuaObjectDescriptor *instance);
                
                
                /**
                 调用Setter方法

                 @param session 会话
                 @param instance 实例对象
                 @param value 属性值
                 */
                virtual void invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value);
                
            public:
                
                /**
                 类型描述
                 */
                LuaExportTypeDescriptor *typeDescriptor;
                
            private:
                
                /**
                 属性名称
                 */
                std::string _name;
                
                /**
                 是否可读
                 */
                bool _canRead;
                
                /**
                 是否可写
                 */
                bool _canWrite;
                
                /**
                 获取处理器
                 */
                LuaFunction *_getter;
                
                /**
                 设置处理器
                 */
                LuaFunction *_setter;
            };
        }
    }
}

#endif /* LuaExportPropertyDescriptor_hpp */
