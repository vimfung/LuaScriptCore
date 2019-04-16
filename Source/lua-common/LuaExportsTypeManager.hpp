//
//  LuaExportsTypeManager.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/16.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaExportsTypeManager_hpp
#define LuaExportsTypeManager_hpp

#include <stdio.h>
#include <map>
#include <string>
#include "LuaObject.h"
#include "lua.hpp"
#include "LuaObjectDescriptor.h"
#include "LuaExportPropertyDescriptor.hpp"
#include "LuaSession.h"
#include "LuaExportTypeDescriptor.hpp"

namespace cn {
    namespace vimfung {
        namespace luascriptcore {
            
            class LuaContext;
            class LuaExportTypeDescriptor;
            class LuaObjectDescriptor;
            class LuaExportPropertyDescriptor;
            class LuaSession;

            /**
             导出类型管理器
             */
            class LuaExportsTypeManager : public LuaObject
            {
            public:
                
                /**
                 初始化
                 
                 @param context 上下文对象
                 @param platform 平台类型
                 */
                LuaExportsTypeManager(LuaContext *context, std::string const& platform);
                
                /**
                 销毁对象
                 */
                virtual ~LuaExportsTypeManager();
                
                /**
                 获取上下文对象
                 
                 @return 上下文对象
                 */
                LuaContext *context();
                
                /**
                 获取类型描述

                 @param name 类型名称
                 @return 类型描述对象
                 */
                LuaExportTypeDescriptor* getExportTypeDescriptor(std::string const& name);

                /**
                 导出类型

                 @param typeDescriptor 类型描述
                 */
                void exportsType(LuaExportTypeDescriptor *typeDescriptor);
                
                /**
                 根据一个原生对象创建一个Lua对象
                 
                 @param objectDescriptor 对象实例
                 */
                void createLuaObject(LuaObjectDescriptor *objectDescriptor);
                
            public:

                /**
                 * 获取平台类型
                 * @return 平台类型
                 */
                std::string getPlatform();

                /**
                 * 映射类型
                 * @param platform 平台类型
                 * @param name  类型名称
                 * @param alias 别名
                 * @return  true 映射成功，否则失败
                 */
                bool _mappingType(std::string const& platform, std::string const& name, std::string const& alias);

                /**
                 * 获取类型完整名称
                 * @param name 映射类型名称
                 * @return 完整的类型名称
                 */
                std::string _getTypeFullName(std::string const& name);

                /**
                 * 创建类型描述
                 * @param name  类型名称
                 * @return 类型描述对象
                 */
                LuaExportTypeDescriptor* _createTypeDescriptor(std::string const& name);

                /**
                 * 获取映射类型
                 * @param name  类型名称
                 * @return 类型描述对象
                 */
                LuaExportTypeDescriptor* _getMappingType(std::string const& name);
                
                /**
                 初始化Lua对象
                 
                 @param objectDescriptor 实例对象
                 */
                void _initLuaObject(LuaObjectDescriptor *objectDescriptor);
                
                /**
                 将原生对象与Lua对象进行绑定
                 
                 @param objectDescriptor 实例对象
                 */
                void _bindLuaInstance(LuaObjectDescriptor *objectDescriptor);
                
                /**
                 准备导出类型
                 
                 @param state 状态
                 @param typeDescriptor 类型描述
                 */
                void _prepareExportsType(lua_State *state, LuaExportTypeDescriptor *typeDescriptor);
                
                
                /**
                 获取实例属性值

                 @param session 会话
                 @param instance 实例
                 @param typeDescriptor 类型
                 @param propertyName 属性名称
                 @return 返回参数数量
                 */
                int _getInstancePropertyValue(LuaSession *session,
                                                LuaObjectDescriptor *instance,
                                                LuaExportTypeDescriptor *typeDescriptor,
                                                std::string const& propertyName);

                /**
                 * 查找实例属性
                 *
                 * @param session 会话
                 * @param typeDescriptor 类型
                 * @param propertyName 属性名称
                 * @return 属性描述
                 */
                LuaExportPropertyDescriptor* _findInstanceProperty(LuaSession *session,
                                                                   LuaExportTypeDescriptor *typeDescriptor,
                                                                   std::string const& propertyName);
                
            private:
                
                /**
                 上下文对象
                 */
                LuaContext *_context;
                
                /**
                 平台类型
                 */
                std::string _platform;

                /**
                 * 导出类型映射，key为类型别名，value为原生类型名称
                 */
                std::map<std::string, std::string> _exportTypesMapping;

                /**
                 导出类型
                 */
                std::map<std::string, LuaExportTypeDescriptor*> _exportTypes;

                /**
                 初始化导出类型
                 */
                void _setupExportType();
                
                /**
                 初始化导出环境
                 */
                void _setupExportEnv();

                /**
                 导出类型

                 @param state 状态
                 @param typeDescriptor 类型描述
                 */
                void _exportsType(lua_State *state, LuaExportTypeDescriptor *typeDescriptor);
                
                /**
                 导出类方法

                 @param state 状态
                 @param typeDescriptor 类型描述
                 */
                void _exportsClassMethods(lua_State *state, LuaExportTypeDescriptor *typeDescriptor);
                
                /**
                 导出实例方法

                 @param state 状态
                 @param typeDescriptor 类型描述
                 */
                void _exportsInstanceMethods(lua_State *state, LuaExportTypeDescriptor *typeDescriptor);

            };
            
        }
    }
}

#endif /* LuaExportsTypeManager_hpp */
