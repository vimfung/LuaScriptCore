//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAVALUE_H
#define SAMPLE_LUAVALUE_H

#include <string>
#include <list>
#include <map>
#include "lua.hpp"
#include "LuaObject.h"
#include "LuaDefined.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaValue;
            class LuaContext;
            class LuaFunction;
            class LuaTuple;
            class LuaPointer;
            class LuaObjectDescriptor;

            /**
             * Lua值，用于Lua与C＋＋中交互数据使用
             */
            class LuaValue : public LuaObject
            {
            private:
                LuaValueType _type;
                lua_Integer _intValue;
                bool _booleanValue;
                double _numberValue;
                size_t _bytesLen;
                void *_value;
                
            public:
                /**
                 * 初始化
                 */
                LuaValue ();

                /**
                 * 初始化, 在反序列化对象时会触发该方法
                 *
                 * @param decoder 解码器
                 */
                LuaValue (LuaObjectDecoder *decoder);

                /**
                 * 初始化
                 *
                 * @param value 整型
                 */
                LuaValue (long value);

                /**
                 * 初始化
                 *
                 * @param value 布尔类型
                 */
                LuaValue (bool value);

                /**
                 * 初始化
                 *
                 * @param value 浮点型
                 */
                LuaValue (double value);

                /**
                 * 初始化
                 *
                 * @param value 字符串
                 */
                LuaValue (std::string value);

                /**
                 * 初始化
                 *
                 * @param bytes 二进制数组
                 * @param length 数组长度
                 *
                 */
                LuaValue (const char *bytes, size_t length);

                /**
                 * 初始化
                 *
                 * @param value LuaValue列表
                 */
                LuaValue (LuaValueList value);

                /**
                 * 初始化
                 *
                 * @param value LuaValue字典
                 */
                LuaValue (LuaValueMap value);

                /**
                 * 初始化
                 *
                 * @param value Lua中的指针
                 */
                LuaValue (LuaPointer *value);

                /**
                 * 初始化
                 *
                 * @param value 对象描述器
                 */
                LuaValue (LuaObjectDescriptor *value);

                /**
                 * 初始化
                 *
                 * @param value Lua中的方法
                 */
                LuaValue (LuaFunction *value);

                /**
                 * 初始化
                 *
                 * @param value 元组
                 */
                LuaValue (LuaTuple *value);

                /**
                 * 析构
                 */
                ~LuaValue();
                
                /**
                 序列化对象
                 
                 @param className 类型名称
                 @param encoder 编码器
                 */
                virtual void serialization (std::string className, LuaObjectEncoder *encoder);
                
            public:

                /**
                 * 获取类型
                 *
                 * @return 类型
                 */
                LuaValueType getType();

                /**
                 * 转换为整数
                 *
                 * @return 整数值
                 */
                long toInteger();

                /**
                 * 转换为字符串
                 *
                 * @return 字符串
                 */
                const std::string toString();

                /**
                 * 转换为浮点数
                 *
                 * @return 浮点数
                 */
                double toNumber();

                /**
                 * 转换为布尔值
                 *
                 * @return 布尔值
                 */
                bool toBoolean();

                /**
                 * 转换为二进制数组
                 *
                 * @return 二进制数组
                 */
                const char* toData();

                /**
                 * 获取二进制数组的长度
                 *
                 * @return 长度
                 */
                size_t getDataLength();

                /**
                 * 转换为数组
                 *
                 * @return 数组
                 */
                LuaValueList* toArray();

                /**
                 * 转换为字典
                 *
                 * @return 字典
                 */
                LuaValueMap* toMap();

                /**
                 * 转换为指针
                 *
                 * @return 指针
                 */
                LuaPointer* toPointer();

                /**
                 * 转换为方法
                 *
                 * @return 方法
                 */
                LuaFunction* toFunction();

                /**
                 * 转换为元组
                 *
                 * @return 元组
                 */
                LuaTuple* toTuple();

                /**
                 * 转换为对象
                 *
                 * @return 对象
                 */
                LuaObjectDescriptor* toObject();

                /**
                 * 入栈数据
                 *
                 * @param context 上下文对象
                 */
                void push(LuaContext *context);

                /**
                 * 将某个LuaValue入栈
                 *
                 * @param context 上下文对象
                 * @param value 值
                 */
                void pushValue(LuaContext *context, LuaValue *value);

                /**
                 * 将某个LuaValue列表入栈
                 *
                 * @param context 上下文对象
                 * @param list 列表
                 */
                void pushTable(LuaContext *context, LuaValueList *list);

                /**
                 * 将某个LuaValue字典入栈
                 *
                 * @param context 上下文对象
                 * @param map 字典
                 */
                void pushTable(LuaContext *context, LuaValueMap *map);

            public:

                /**
                 * 创建一个空值对象
                 *
                 * @return 值对象
                 */
                static LuaValue* NilValue();

                /**
                 * 创建一个整型值对象
                 *
                 * @param value 整数
                 *
                 * @return 值对象
                 */
                static LuaValue* IntegerValue(long value);

                /**
                 * 创建一个布尔值对象
                 *
                 * @param value 布尔值
                 *
                 * @return 值对象
                 */
                static LuaValue* BooleanValue(bool value);

                /**
                 * 创建一个浮点数值对象
                 *
                 * @param value 浮点数
                 *
                 * @return 值对象
                 */
                static LuaValue* NumberValue(double value);

                /**
                 * 创建一个字符串值对象
                 *
                 * @param value 字符串
                 *
                 * @return 值对象
                 */
                static LuaValue* StringValue(std::string value);

                /**
                 * 创建一个二进制数组值对象
                 *
                 * @param bytes 二进制数组
                 * @param length 数组长度
                 *
                 * @return 值对象
                 */
                static LuaValue* DataValue(const char *bytes, size_t length);

                /**
                 * 创建一个数组值对象
                 *
                 * @param value 数组
                 *
                 * @return 值对象
                 */
                static LuaValue* ArrayValue(LuaValueList value);

                /**
                 * 创建一个字典值对象
                 *
                 * @param value 字典
                 *
                 * @return 值对象
                 */
                static LuaValue* DictonaryValue(LuaValueMap value);

                /**
                 * 创建一个指针值对象
                 *
                 * @param value 指针
                 *
                 * @return 值对象
                 */
                static LuaValue* PointerValue(LuaPointer *value);

                /**
                 * 创建一个方法值对象
                 *
                 * @param value 方法
                 *
                 * @return 值对象
                 */
                static LuaValue* FunctionValue(LuaFunction *value);

                /**
                 * 创建一个元组值对象
                 *
                 * @param value 元组
                 *
                 * @return 值对象
                 */
                static LuaValue* TupleValue(LuaTuple *value);

                /**
                 * 创建一个对象值对象
                 *
                 * @param value 对象描述器
                 *
                 * @return 值对象
                 */
                static LuaValue* ObjectValue(LuaObjectDescriptor *value);
            };
            
        }
    }
}

#endif //SAMPLE_LUAVALUE_H
