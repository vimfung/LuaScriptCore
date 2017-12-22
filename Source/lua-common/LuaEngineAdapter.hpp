//
//  LuaEngineAdapter.hpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/8/17.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#ifndef LuaEngineAdapter_hpp
#define LuaEngineAdapter_hpp

#include <stdio.h>
#include "lua.hpp"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            /**
             * 引擎适配器
             **/
            class LuaEngineAdapter
            {
            public:
                
                /**
                 * 创建新的Lua状态对象
                 *
                 * @return Lua状态对象
                 **/
                static lua_State* newState();
                
                /**
                 * 调用垃圾回收
                 * 
                 * @param state 状态对象
                 * @param what 操作
                 * @param data 数据
                 *
                 * @return 执行结果
                 **/
                static int GC(lua_State *state, int what, int data);
                
                /**
                 * 加载标准库
                 *
                 * @param state 状态对象
                 **/
                static void openLibs (lua_State *state);
                
                /**
                 * 释放状态对象
                 *
                 * @param state 状态对象
                 **/
                static void close (lua_State *state);
                
                /**
                 * 转换栈中指定数据为Userdata类型
                 *
                 * @param state 状态对象
                 * @param idx 栈索引
                 *
                 * @return Userdata数据
                 **/
                static void* toUserdata (lua_State *state, int idx);
                
                /**
                 * 转换为闭包环境变量索引
                 *
                 * @param index 索引
                 * @return 闭包索引
                 **/
                static int upValueIndex (int index);
                
                /**
                 * 转换指定栈数据为字符串类型
                 *
                 * @param state 状态对象
                 * @param idx 栈索引
                 *
                 * @return 字符串
                 **/
                static const char* toString (lua_State *state, int idx);
                
                /**
                 * 获取全局变量
                 *
                 * @param state 状态对象
                 * @param name 名称
                 **/
                static void getGlobal (lua_State *state, const char *name);
                
                /**
                 * 获取Table的字段值，并放入栈中
                 *
                 * @param state 状态对象
                 * @param tblIndex Table在栈中位置
                 * @param key 字段名称
                 **/
                static void getField (lua_State *state, int tblIndex, const char *key);
                
                /**
                 出栈

                 @param state 状态对象
                 @param count 出栈元素数量
                 */
                static void pop(lua_State *state, int count);
                
                /**
                 入栈一个字符串

                 @param state 状态对象
                 @param str 字符串
                 */
                static void pushString (lua_State *state, const char *str);
                
                /**
                 入栈一个字符串

                 @param state 状态对象
                 @param str 字符串
                 @param len 长度
                 */
                static void pushString (lua_State *state, const char *str, size_t len);
                
                /**
                 设置字段

                 @param state 状态对象
                 @param tblIndex Table的栈位置
                 @param key 字段名称
                 */
                static void setField (lua_State *state, int tblIndex, const char *key);
                
                /**
                 设置全局变量

                 @param state 状态对象
                 @param name 变量名称
                 */
                static void setGlobal (lua_State *state, const char *name);
                
                /**
                 获取栈顶位置

                 @param state 状态对象
                 @return 栈顶位置索引
                 */
                static int getTop (lua_State *state);
                
                /**
                 加载并解析字符串

                 @param state 状态对象
                 @param str 字符串
                 @return 执行状态
                 */
                static int loadString (lua_State *state, const char *str);
                
                /**
                 调用方法

                 @param state 状态对象
                 @param argsCount 参数数量
                 @param resultsCount 返回结果数量
                 @param errfunc 错误方法
                 @return 执行状态
                 */
                static int pCall (lua_State *state, int argsCount, int resultsCount, int errfunc);
                
                /**
                 加载并解析文件

                 @param state 状态对象
                 @param filename 文件名称
                 @return 执行状态
                 */
                static int loadFile (lua_State *state, const char *filename);
                
                /**
                 判断是否为Function类型

                 @param state 状态对象
                 @param index 栈索引
                 @return true 是Function类型，否则不是。
                 */
                static bool isFunction(lua_State *state, int index);
                
                /**
                 入栈一个轻量级Userdata

                 @param state 状态
                 @param pointer 指针
                 */
                static void pushLightUserdata (lua_State *state, void *pointer);
                
                /**
                 入栈一个C闭包

                 @param state 状态
                 @param fn 方法
                 @param n 方法携带的环境变量数量
                 */
                static void pushCClosure (lua_State *state, lua_CFunction fn, int n);
                
                
                /**
                 入栈一个C函数

                 @param state 状态
                 @param fn 方法
                 */
                static void pushCFunction (lua_State *state, lua_CFunction fn);
                
                /**
                 判断是否为nil

                 @param state 状态对象
                 @param index 栈索引
                 @return true 是nil，否则不是
                 */
                static bool isNil(lua_State *state, int index);
                
                /**
                 入栈一个Nil类型

                 @param state 状态对象
                 */
                static void pushNil (lua_State *state);
                
                /**
                 转换栈索引为一个正数

                 @param state 状态对象
                 @param idx 索引
                 @return 正数索引
                 */
                static int absIndex (lua_State *state, int idx);
                
                /**
                 获取栈中数据类型

                 @param state 状态对象
                 @param idx 索引
                 @return 数据类型
                 */
                static int type (lua_State *state, int idx);
                
                /**
                 转换栈中数据为布尔类型

                 @param state 状态对象
                 @param idx 索引
                 @return bool数据
                 */
                static int toBoolean (lua_State *state, int idx);
                
                /**
                 转换栈中数据为数值类型

                 @param state 状态对象
                 @param idx 索引
                 @return 数值数据
                 */
                static lua_Number toNumber(lua_State *state, int idx);
                
                /**
                 转换栈中数据为字符串

                 @param state 状态对象
                 @param idx 索引
                 @param len 长度
                 @return 字符串
                 */
                static const char* toLString(lua_State *state, int idx, size_t *len);
                
                /**
                 获取一下组Key-Value

                 @param state 状态对象
                 @param idx 索引
                 @return 执行结果
                 */
                static int next(lua_State *state, int idx);
                
                /**
                 转换为指针

                 @param state 状态对象
                 @param idx 索引
                 @return 指针对象
                 */
                static const void* toPointer(lua_State *state, int idx);
                
                /**
                 将栈中某个数据拷贝放入栈顶

                 @param state 状态对象
                 @param idx 索引
                 */
                static void pushValue(lua_State *state, int idx);

                /**
                 入栈一个整数

                 @param state 状态对象
                 @param n 索引
                 */
                static void pushInteger(lua_State *state, lua_Integer n);
                
                /**
                 将栈顶数据插入指定位子

                 @param state 状态对象
                 @param idx 索引
                 */
                static void insert (lua_State *state, int idx);
                
                /**
                 判断是否为Table类型

                 @param state 状态对象
                 @param idx 索引
                 @return true 是，否则不是。
                 */
                static bool isTable(lua_State *state, int idx);
                
                /**
                 创建Table对象

                 @param state 状态对象
                 */
                static void newTable(lua_State *state);
                
                /**
                 设置元表

                 @param state 状态对象
                 @param objindex 对象所在栈索引
                 @return 执行结果
                 */
                static int setMetatable (lua_State *state, int objindex);
                
                /**
                 设置字段值

                 @param state 状态
                 @param idx Table的栈索引
                 @param n 下标
                 */
                static void rawSetI(lua_State *state, int idx, int n);
                
                /**
                 获取表数据的源操作，不出发index元方法
                 
                 @param state 状态
                 @param index table对象位置索引
                 */
                static void rawGet(lua_State *state, int index);
                
                /**
                 入栈一个数值

                 @param state 状态对象
                 @param n 数值
                 */
                static void pushNumber(lua_State *state, lua_Number n);
                
                /**
                 转换为整型数值

                 @param state 状态对象
                 @param idx 索引
                 @return 整型值
                 */
                static lua_Integer toInteger(lua_State *state, int idx);
                
                /**
                 入栈一个布尔值

                 @param state 状态对象
                 @param b 布尔值
                 */
                static void pushBoolean (lua_State *state, int b);
                
                /**
                 判断是否为Userdata类型

                 @param state 状态对象
                 @param idx 索引
                 @return true 是，否则不是
                 */
                static int isUserdata (lua_State *state, int idx);
                
                /**
                 创建Userdata

                 @param state 状态对象
                 @param size 内存大小
                 @return Userdata对象
                 */
                static void* newUserdata (lua_State *state, size_t size);
                
                /**
                 移除栈中指定位置数据

                 @param state 状态对象
                 @param idx 位置索引
                 */
                static void remove (lua_State *state, int idx);
                
                /**
                 获取元表

                 @param state 状态对象
                 @param objindex 位置索引
                 @return 执行结果
                 */
                static int getMetatable (lua_State *state, int objindex);
                
                /**
                 获取元表

                 @param state 状态对象
                 @param tname 表名
                 */
                static void getMetatable (lua_State *state, const char *tname);
                
                /**
                 设置字段值

                 @param state 状态对象
                 @param idx 索引
                 */
                static void rawSet (lua_State *state, int idx);
                
                /**
                 检测指定位置下的数据是否为字符串，如果不是则进行转换

                 @param state 状态
                 @param idx  位置索引
                 @return 字符串
                 */
                static const char* checkString (lua_State *state, int idx);
                
                /**
                 创建元表

                 @param state 状态对象
                 @param tname 元表名称
                 @return 执行结果
                 */
                static int newMetatable (lua_State *state, const char *tname);
                
                /**
                 抛出异常

                 @param state 状态对象
                 @param message 错误消息
                 @return 执行结果
                 */
                static int error (lua_State *state, const char *message);
            };
            
        }
    }
}

#endif /* LuaEngineAdapter_hpp */
