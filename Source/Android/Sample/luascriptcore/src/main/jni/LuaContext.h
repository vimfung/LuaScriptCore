//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUACONTEXT_H
#define SAMPLE_LUACONTEXT_H

#include "lua/lua.hpp"
#include "LuaObject.h"
#include "LuaValue.h"
#include <string>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            typedef void (*LuaExceptionHandler) (std::string message);

            class LuaContext : public LuaObject
            {
            private:
                lua_State *_state;
                LuaExceptionHandler _exceptionHandler;

            private:
                LuaValue* getValueByIndex(int index);

            public:
                LuaContext();
                ~LuaContext();

            public:
                void onException (LuaExceptionHandler handler);

            public:
                LuaValue* evalScript(std::string script);
            };

        }
    }
}


#endif //SAMPLE_LUACONTEXT_H
