//
// Created by 冯鸿杰 on 16/9/27.
//

#ifndef ANDROID_LUADEFINED_H
#define ANDROID_LUADEFINED_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include <deque>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaContext;
            class LuaModule;
            class LuaValue;
            class LuaObject;

            enum LuaValueType
            {
                LuaValueTypeNil = 0,
                LuaValueTypeNumber = 1,
                LuaValueTypeBoolean = 2,
                LuaValueTypeString = 3,
                LuaValueTypeArray = 4,
                LuaValueTypeMap = 5,
                LuaValueTypePtr = 6,
                LuaValueTypeObject = 7,
                LuaValueTypeInteger = 8,
                LuaValueTypeData = 9,
                LuaValueTypeFunction = 10,
                LuaValueTypeTuple = 11,
                LuaValueTypeClass = 12
            };

            /**
             * Userdata引用
             */
            typedef struct {

                void *value;

            }LuaUserdata, *LuaUserdataRef;

            typedef void (*LuaExceptionHandler) (LuaContext *context, std::string message);
            typedef std::deque<LuaValue *> LuaArgumentList;
            typedef std::deque<LuaValue *> LuaValueList;
            typedef std::map<std::string, LuaValue*> LuaValueMap;
            typedef std::map<int, LuaObject*> LuaObjectMap;

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
