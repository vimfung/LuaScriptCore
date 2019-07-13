//
//  LuaTmpValue.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/12/21.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaTmpValue_hpp
#define LuaTmpValue_hpp

#include <stdio.h>
#include "LuaValue.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;
            
            /**
             临时值
             */
            class LuaTmpValue : public LuaValue
            {
            private:
                
                /**
                 索引位置
                 */
                int _index;
                
                /**
                 解析后值对象
                 */
                LuaValue *_parsedValue;
                
            private:
                
                /**
                 解析值
                 */
                void _parseValue();
                
            public:
                
                /**
                 初始化
                 
                 @param context 上下文对象
                 @param index 栈索引
                 */
                LuaTmpValue(LuaContext *context, int index);
                
                /**
                 销毁对象
                 */
                virtual ~LuaTmpValue();
                
                /**
                 序列化对象
                 
                 @param encoder 编码器
                 */
                virtual void serialization (LuaObjectEncoder *encoder);
                
            public:
                
                /**
                 * 获取类型
                 *
                 * @return 类型
                 */
                virtual LuaValueType getType();
                
                /**
                 * 转换为整数
                 *
                 * @return 整数值
                 */
                virtual lua_Integer toInteger();
                
                /**
                 * 转换为字符串
                 *
                 * @return 字符串
                 */
                virtual const std::string toString();
                
                /**
                 * 转换为浮点数
                 *
                 * @return 浮点数
                 */
                virtual double toNumber();
                
                /**
                 * 转换为布尔值
                 *
                 * @return 布尔值
                 */
                virtual bool toBoolean();
                
                /**
                 * 转换为二进制数组
                 *
                 * @return 二进制数组
                 */
                virtual const char* toData();
                
                /**
                 * 获取二进制数组的长度
                 *
                 * @return 长度
                 */
                virtual size_t getDataLength();
                
                /**
                 * 转换为数组
                 *
                 * @return 数组
                 */
                virtual LuaValueList* toArray();
                
                /**
                 * 转换为字典
                 *
                 * @return 字典
                 */
                virtual LuaValueMap* toMap();
                
                /**
                 * 转换为指针
                 *
                 * @return 指针
                 */
                virtual LuaPointer* toPointer();
                
                /**
                 * 转换为方法
                 *
                 * @return 方法
                 */
                virtual LuaFunction* toFunction();
                
                /**
                 * 转换为元组
                 *
                 * @return 元组
                 */
                virtual LuaTuple* toTuple();
                
                /**
                 * 转换为对象
                 *
                 * @return 对象
                 */
                virtual LuaObjectDescriptor* toObject();
                
                /**
                 * 转换为导出Lua类型
                 *
                 * @return 导出Lua类型
                 */
                virtual LuaExportTypeDescriptor* toType();
                
                /**
                 * 入栈数据
                 *
                 * @param context 上下文对象
                 */
                virtual void push(LuaContext *context);
            };
        }
    }
}

#endif /* LuaTmpValue_hpp */
