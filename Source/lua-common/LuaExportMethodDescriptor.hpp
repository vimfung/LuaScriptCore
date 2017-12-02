//
//  LuaExportMethodDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/16.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaExportMethodDescriptor_hpp
#define LuaExportMethodDescriptor_hpp

#include <stdio.h>
#include <string>
#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {
            
            class LuaValue;
            class LuaSession;
            class LuaExportTypeDescriptor;
            
            /**
             方法描述器
             */
            class LuaExportMethodDescriptor : public LuaObject
            {
            public:
                
                /**
                 初始化
                 
                 @param methodSignature 方法签名
                 */
                LuaExportMethodDescriptor(std::string name, std::string methodSignature);
                
                
                /**
                 获取方法名称

                 @return 名称
                 */
                std::string name();
                
                /**
                 获取方法签名

                 @return 方法签名
                 */
                std::string methodSignature();
                
                /**
                 调用方法
                 
                 @param session 会话
                 @param arguments 参数列表
                 @return 返回值
                 */
                virtual LuaValue* invoke(LuaSession *session, LuaArgumentList arguments);
                
            public:
                
                /**
                 类型描述
                 */
                LuaExportTypeDescriptor *typeDescriptor;
                
            private:
                
                /**
                 方法名称
                 */
                std::string _name;
                
                /**
                 方法签名
                 */
                std::string _methodSignature;
            };
            
        }
    }
}

#endif /* LuaExportMethodDescriptor_hpp */
