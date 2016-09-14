//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUACONTEXT_H
#define SAMPLE_LUACONTEXT_H

#include "lua.hpp"
#include "LuaObject.h"
#include "LuaValue.h"
#include <string>
#include <map>
#include <list>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;

            typedef void (*LuaExceptionHandler) (std::string message);
            typedef std::list<LuaValue *> LuaArgumentList;
            typedef LuaValue* (*LuaMethodHandler) (LuaContext *context, std::string methodName, LuaArgumentList arguments);
            typedef std::map<std::string, LuaMethodHandler> LuaMethodMap;

            class LuaContext : public LuaObject
            {
            private:
                lua_State *_state;
                LuaExceptionHandler _exceptionHandler;
                LuaMethodMap _methodMap;

            public:
                LuaContext();
                ~LuaContext();

            public:
                void onException (LuaExceptionHandler handler);

            public:
                LuaValue* evalScript(std::string script);
                LuaValue* evalScriptFromFile(std::string path);
                LuaValue* callMethod(std::string methodName, LuaArgumentList arguments);
                void registerMethod(std::string methodName, LuaMethodHandler handler);

            public:
                //获取方法处理器,方法名称
                LuaMethodHandler getMethodHandler(std::string methodName);
                LuaValue* getValueByIndex(int index);
            };

        }
    }
}


#endif //SAMPLE_LUACONTEXT_H
