//
// Created by 冯鸿杰 on 16/9/27.
//

#ifndef ANDROID_LUADEFINED_H
#define ANDROID_LUADEFINED_H

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
            class LuaModule;
            class LuaValue;

            typedef void (*LuaExceptionHandler) (LuaContext *context, std::string message);
            typedef std::list<LuaValue *> LuaArgumentList;

            typedef LuaValue* (*LuaMethodHandler) (LuaContext *context, std::string methodName, LuaArgumentList arguments);
            typedef LuaValue* (*LuaModuleMethodHandler) (LuaModule *module, std::string methodName, LuaArgumentList arguments);
            typedef LuaValue* (*LuaModuleGetterHandler) (LuaModule *module, std::string fieldName);
            typedef void (*LuaModuleSetterHandler) (LuaModule *module, std::string fieldName, LuaValue *value);

            typedef std::map<std::string, LuaModuleMethodHandler> LuaModuleMethodMap;
            typedef std::map<std::string, LuaMethodHandler> LuaMethodMap;
            typedef std::map<std::string, LuaModuleSetterHandler> LuaModuleSetterMap;
            typedef std::map<std::string, LuaModuleGetterHandler> LuaModuleGetterMap;
            typedef std::map<std::string, LuaModule*> LuaModuleMap;
        }
    }
}



#endif //ANDROID_LUADEFINED_H
