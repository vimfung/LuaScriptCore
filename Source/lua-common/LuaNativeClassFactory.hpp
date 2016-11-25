//
//  LuaNativeClassFactory.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef LuaNativeClassFactory_hpp
#define LuaNativeClassFactory_hpp

#include <stdio.h>
#include <map>
#include <string>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaNativeClass;
            
            typedef std::map<std::string, LuaNativeClass*> LuaClassMap;
            
            class LuaNativeClassFactory
            {
            private:
                
                LuaClassMap _classMap;
                
            public:
                static LuaNativeClassFactory& shareInstance();
                
            public:
                
                /**
                 注册类型
                 
                 @param className 类名称
                 @param nativeClass 类型
                 */
                void registerClass(std::string className, LuaNativeClass* nativeClass);
                
                
                /**
                 查找类型
                 
                 @param className 类名称
                 @return 类型
                 */
                LuaNativeClass* findClass(std::string className);
                
            };
            
        }
    }
}

#endif /* LuaNativeClassFactory_hpp */
