//
//  LuaTypeDescriptor.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/16.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaTypeDescriptor_hpp
#define LuaTypeDescriptor_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <list>
#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {
            
            class LuaExportMethodDescriptor;
            class LuaExportPropertyDescriptor;
            class LuaObjectDescriptor;
            class LuaSession;
            
            typedef std::list<LuaExportMethodDescriptor *> MethodList;
            typedef std::map<std::string, MethodList> MethodMap;
            typedef std::map<std::string, LuaExportMethodDescriptor *> MappingMethodMap;
            typedef std::map<std::string, LuaExportPropertyDescriptor *> PropertyMap;
            
            /**
             Lua类型描述
             */
            class LuaExportTypeDescriptor : public LuaObject
            {
            public:
                
                /**
                 初始化
                 
                 @param typeName 类型名称
                 @param parentTypeDescriptor 父级类型
                 */
                LuaExportTypeDescriptor (std::string &typeName, LuaExportTypeDescriptor *parentTypeDescriptor);
                
                /**
                 析构对象
                 */
                ~LuaExportTypeDescriptor();
                
                /**
                 获取类型名称
                 
                 @return 类型名称
                 */
                std::string typeName();
                
                /**
                 获取原型类型名称
                 
                 @return 原型类型名称
                 */
                std::string prototypeTypeName();
                
                /**
                 获取类方法名称列表

                 @return 方法名称列表
                 */
                std::list<std::string> classMethodNameList();
                
                /**
                 获取实例方法名称列表

                 @return 方法名称列表
                 */
                std::list<std::string> instanceMethodNameList();
                
                /**
                 获取父级类型描述对象

                 @return 父级类型描述对象
                 */
                LuaExportTypeDescriptor* parentTypeDescriptor();
                
                /**
                 是否为指定类型的子类

                 @param typeDescriptor 类型
                 @return true 为子类，否则不是
                 */
                bool subtypeOfType(LuaExportTypeDescriptor *typeDescriptor);
                
                /**
                 添加类方法

                 @param methodName 类方法名称
                 @param methodDescriptor 方法描述对象
                 */
                void addClassMethod(std::string methodName, LuaExportMethodDescriptor *methodDescriptor);
                
                /**
                 添加实例方法

                 @param methodName 方法名称
                 @param methodDescriptor 方法描述对象
                 */
                void addInstanceMethod(std::string methodName, LuaExportMethodDescriptor *methodDescriptor);
                
                /**
                 添加属性
                 
                 @param propertyName 属性名称
                 @param propertyDescriptor 属性描述对象
                 */
                void addProperty(std::string propertyName, LuaExportPropertyDescriptor *propertyDescriptor);
                
                /**
                 获取类方法

                 @param methodName 方法名称
                 @param arguments 传入参数
                 @return 方法描述
                 */
                LuaExportMethodDescriptor* getClassMethod(std::string methodName, LuaArgumentList arguments);
                
                /**
                 获取实例方法

                 @param methodName 方法名称
                 @param arguments 传入参数
                 @return 方法描述
                 */
                LuaExportMethodDescriptor* getInstanceMethod(std::string methodName, LuaArgumentList arguments);
                
                /**
                 获取属性
                 
                 @param propertyName 属性名称
                 @return 属性对象
                 */
                LuaExportPropertyDescriptor* getProperty(std::string propertyName);
                
            public:
                
                /**
                 创建实例
                 
                 @param session 会话
                 @return 实例对象
                 */
                virtual LuaObjectDescriptor* createInstance(LuaSession *session);
                
                /**
                 销毁实例

                 @param session 会话
                 @param objectDescriptor 实例对象
                 */
                virtual void destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor);
                
                /**
                 创建子类型
                 
                 @param session 会话
                 @param subTypeName 子类型名称
                 @return 类型
                 */
                virtual LuaExportTypeDescriptor* createSubType(LuaSession *session, std::string subTypeName);
                
            private:
                
                /**
                 类型名称
                 */
                std::string _typeName;
                
                /**
                 原型类型名称
                 */
                std::string _prototypeTypeName;
                
                /**
                 类方法集合
                 */
                MethodMap _classMethods;
                
                /**
                 实例方法集合
                 */
                MethodMap _instanceMethods;
                
                /**
                 属性集合
                 */
                PropertyMap _properties;
                
                /**
                 父类型描述
                 */
                LuaExportTypeDescriptor *_parentTypeDescriptor;
                
                /**
                 方法映射表，用于提供匹配重载方法的效率，第一次调用方法时会将最匹配的方法记录入该表，以后可从该表中取出对应的方法。
                 */
                MappingMethodMap _methodsMapping;
                
                /**
                 过滤方法
                 
                 @param methodName 方法名称
                 @param arguments 传入参数
                 @param isStatic 是否为静态方法
                 @return 方法描述
                 */
                LuaExportMethodDescriptor* filterMethod(std::string methodName, LuaArgumentList arguments, bool isStatic);
            };
            
        }
    }
}

#endif /* LuaTypeDescriptor_hpp */
