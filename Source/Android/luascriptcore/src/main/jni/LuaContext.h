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
            class LuaContext : public LuaObject
            {
            private:
                lua_State *_state;
                LuaExceptionHandler _exceptionHandler;
                LuaMethodMap _methodMap;
                LuaModuleMap _moduleMap;

            public:
                LuaContext();
                ~LuaContext();

            public:
                void onException (LuaExceptionHandler handler);

            public:
                void addSearchPath(std::string path);
                LuaValue* evalScript(std::string script);
                LuaValue* evalScriptFromFile(std::string path);
                LuaValue* callMethod(std::string methodName, LuaArgumentList *arguments);
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
                //获取方法处理器,方法名称
                LuaMethodHandler getMethodHandler(std::string methodName);
                LuaValue* getValueByIndex(int index);
                lua_State* getLuaState();

            };

        }
    }
}


#endif //SAMPLE_LUACONTEXT_H
