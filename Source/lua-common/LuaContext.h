//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUACONTEXT_H
#define SAMPLE_LUACONTEXT_H

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
            class LuaModule;
            class LuaDataExchanger;

            /**
             * Lua上下文环境, 维护原生代码与Lua之间交互的核心类型。
             */
            class LuaContext : public LuaObject
            {
            private:

                /**
                 * Lua状态机
                 */
                lua_State *_state;

                /**
                 * Lua运行异常处理器
                 */
                LuaExceptionHandler _exceptionHandler;

                /**
                 * 方法映射表
                 */
                LuaMethodMap _methodMap;

                /**
                 * 模块映射表
                 */
                LuaModuleMap _moduleMap;

                /**
                 * 数据交换器
                 */
                LuaDataExchanger *_dataExchanger;

            public:

                /**
                 * 初始化上下文对象
                 */
                LuaContext();

                /**
                 * 销毁上下文对象
                 */
                ~LuaContext();

            public:

                /**
                 * 当lua执行异常时触发该事件
                 *
                 * @param handler 事件处理器
                 */
                void onException (LuaExceptionHandler handler);

                /**
                 * 抛出异常信息
                 *
                 * @param message 异常消息
                 */
                void raiseException (std::string message);

            public:

                /**
                 * 添加搜索路径
                 *
                 * @param path 路径
                 */
                void addSearchPath(std::string path);

                /**
                 * 设置全局变量
                 *
                 * @param name 变量名称
                 * @param value 变量值
                 */
                void setGlobal(std::string name, LuaValue *value);

                /**
                 * 获取全局变量
                 *
                 * @param name 变量名称
                 *
                 * @return 变量值
                 */
                LuaValue* getGlobal(std::string name);

                /**
                 * 保留Lua层的变量引用，使其不被GC所回收。
                 * 注：判断value能否被保留取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
                 * 即：LuaValue *val1 = new LuaValue(obj1)与LuaValue *val2 = new LuaValue(obj1)传入方法中效果相同。
                 *
                 * @param value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。
                 */
                void retainValue(LuaValue *value);

                /**
                 * 释放Lua层的变量引用，使其内存管理权交回Lua。
                 * 注：判断value能否被释放取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
                 * 即：LuaValue *val1 = new LuaValue(obj1)与LuaValue *val2 = new LuaValue(obj1)传入方法中效果相同。
                 *
                 * @param value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。
                 */
                void releaseValue(LuaValue *value);

                /**
                 * 解析脚本
                 *
                 * @param script 脚本内容
                 */
                LuaValue* evalScript(std::string script);

                /**
                 * 从lua文件中解析脚本
                 *
                 * @param path lua文件路径
                 */
                LuaValue* evalScriptFromFile(std::string path);

                /**
                 * 调用方法
                 *
                 * @param methodName 方法名称
                 * @param arguments 参数列表
                 */
                LuaValue* callMethod(std::string methodName, LuaArgumentList *arguments);

                /**
                 * 注册方法
                 *
                 * @param methodName 方法名称
                 * @param handler 方法处理
                 */
                void registerMethod(std::string methodName, LuaMethodHandler handler);

                /**
                 * 注册模块
                 *
                 * @param moduleName 模块名称
                 * @param module    模块实例对象
                 */
                void registerModule(const std::string &moduleName, LuaModule *module);

                /**
                 * 判断模块是否注册
                 *
                 * @param moduleName 模块名称
                 *
                 * @return true 已注册, false 尚未注册
                 */
                bool isModuleRegisted(const std::string &moduleName);

                /**
                 * 根据模块名称获取模块对象
                 *
                 * @param moduleName 模块名称
                 *
                 * @return 模块对象，如果不存在则返回NULL
                 */
                LuaModule* getModule(const std::string &moduleName);

            public:

                /**
                 * 根据方法名称获取对应的方法处理器
                 *
                 * @param methodName 方法名称
                 *
                 * @return 方法处理器
                 */
                LuaMethodHandler getMethodHandler(std::string methodName);

                /**
                 * 获取Lua状态机
                 *
                 * @return Lua状态机
                 */
                lua_State* getLuaState();

                /**
                 * 获取数据数据交换层
                 */
                LuaDataExchanger *getDataExchanger();

            };

        }
    }
}


#endif //SAMPLE_LUACONTEXT_H
