//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUACONTEXT_H
#define SAMPLE_LUACONTEXT_H

#include "lua.hpp"
#include "LuaObject.h"
#include "LuaValue.h"
#include "LuaModule.h"
#import "LuaDefined.h"

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
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
                 * 获取数据栈中对应索引的值
                 *
                 * @param index 数据栈索引
                 *
                 * @return 值对象
                 */
                LuaValue* getValueByIndex(int index);

                /**
                 * 获取Lua状态机
                 *
                 * @return Lua状态机
                 */
                lua_State* getLuaState();

            };

        }
    }
}


#endif //SAMPLE_LUACONTEXT_H
