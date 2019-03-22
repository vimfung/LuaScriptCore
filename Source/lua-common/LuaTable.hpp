//
//  LuaTable.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/1/18.
//  Copyright © 2019年 冯鸿杰. All rights reserved.
//

#ifndef LuaTable_hpp
#define LuaTable_hpp

#include <stdio.h>
#include "LuaManagedObject.h"
#include "LuaDefined.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;
            
            /**
             table类型，对应lua中的table
             */
            class LuaTable : public LuaManagedObject
            {
            public:
                
                /**
                 初始化
                 
                 @param map 字典结构
                 @param exchangeId 对象标识
                 @param context 上下文对象
                 */
                LuaTable(LuaValueMap map, std::string exchangeId, LuaContext *context);
                
                /**
                 初始化
                 
                 @param list 数组结构
                 @param exchangeId 对象标识
                 @param context 上下文对象
                 */
                LuaTable(LuaValueList list, std::string exchangeId, LuaContext *context);
                
                /**
                 对象析构
                 */
                ~LuaTable();
                
            private:
                
                bool _isArray;
                void *_valueObject;
                
            private:
                
                
                /**
                 设置指定键对象

                 @param map 字典
                 @param keys 键名数组，带层级关系
                 @param keyIndex 键名在数组中的索引
                 @param object 对象
                 */
                void setObject(LuaValueMap *map,
                               std::deque<std::string> keys,
                               int keyIndex,
                               LuaValue *object);
                
            public:
                
                /**
                 判断是否为数组

                 @return true 数组 false 字典
                 */
                bool isArray();
                
                /**
                 获取值对象

                 @return 值对象
                 */
                void* getValueObject();
                
                /**
                 设置指定键对象

                 @param keyPath 键名路径
                 @param object 对象
                 */
                void setObject(std::string keyPath, LuaValue *object);
                
            public:
                
                /**
                 * 入栈数据
                 *
                 * @param context 上下文对象
                 */
                virtual void push(LuaContext *context);
                
                /**
                 * 入栈数据
                 *
                 * @param state lua状态
                 * @param queue 队列
                 */
                virtual void push(lua_State *state, LuaOperationQueue *queue);
                
            };
        }
    }
}

#endif /* LuaTable_hpp */
