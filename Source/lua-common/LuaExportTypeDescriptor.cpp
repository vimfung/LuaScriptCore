//
//  LuaTypeDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/16.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaExportTypeDescriptor.hpp"
#include "LuaExportMethodDescriptor.hpp"
#include "LuaExportPropertyDescriptor.hpp"
#include "StringUtils.h"
#include "LuaSession.h"
#include "LuaObjectDescriptor.h"
#include "LuaEngineAdapter.hpp"
#include "LuaContext.h"
#include "LuaExportsTypeManager.hpp"
#include "LuaValue.h"
#include <deque>
#include <regex>

using namespace cn::vimfung::luascriptcore;

LuaExportTypeDescriptor::LuaExportTypeDescriptor (std::string &typeName, LuaExportTypeDescriptor *parentTypeDescriptor)
{
    _typeName = typeName;
    _parentTypeDescriptor = parentTypeDescriptor;
    _prototypeTypeName = StringUtils::format("_%s_PROTOTYPE_", typeName.c_str());
}

LuaExportTypeDescriptor::~LuaExportTypeDescriptor()
{
    //释放类方法
    for (MethodMap::iterator it = _classMethods.begin(); it != _classMethods.end(); it++)
    {
        MethodList methodList = it -> second;
        for (MethodList::iterator mit = methodList.begin(); mit != methodList.end(); mit++)
        {
            (*mit) -> release();
        }
    }
    
    //释放实例方法
    for (MethodMap::iterator it = _instanceMethods.begin(); it != _instanceMethods.end(); it++)
    {
        MethodList methodList = it -> second;
        for (MethodList::iterator mit = methodList.begin(); mit != methodList.end(); mit++)
        {
            (*mit) -> release();
        }
    }
    
    //释放属性
    for (PropertyMap::iterator it = _properties.begin(); it != _properties.end(); it++)
    {
        it -> second -> release();
    }
}

std::string LuaExportTypeDescriptor::typeName()
{
    return _typeName;
}

std::string LuaExportTypeDescriptor::prototypeTypeName()
{
    return _prototypeTypeName;
}

LuaExportTypeDescriptor* LuaExportTypeDescriptor::parentTypeDescriptor()
{
    return _parentTypeDescriptor;
}

bool LuaExportTypeDescriptor::subtypeOfType(LuaExportTypeDescriptor *typeDescriptor)
{
    LuaExportTypeDescriptor *targetTypeDescriptor = this;
    while (targetTypeDescriptor != NULL)
    {
        if (targetTypeDescriptor == typeDescriptor)
        {
            return true;
        }
        
        targetTypeDescriptor = targetTypeDescriptor -> parentTypeDescriptor();
    }
    
    return false;
}

void LuaExportTypeDescriptor::addClassMethod(std::string methodName, LuaExportMethodDescriptor *methodDescriptor)
{
    std::list<LuaExportMethodDescriptor *> methods = _classMethods[methodName];
    methodDescriptor -> retain();
    methodDescriptor -> typeDescriptor = this;
    methods.push_back(methodDescriptor);
    _classMethods[methodName] = methods;
}

void LuaExportTypeDescriptor::addInstanceMethod(std::string methodName, LuaExportMethodDescriptor *methodDescriptor)
{
    std::list<LuaExportMethodDescriptor *> methods = _instanceMethods[methodName];
    methodDescriptor -> retain();
    methodDescriptor -> typeDescriptor = this;
    methods.push_back(methodDescriptor);
    _instanceMethods[methodName] = methods;
}

void LuaExportTypeDescriptor::addProperty(std::string propertyName, LuaExportPropertyDescriptor *propertyDescriptor)
{
    propertyDescriptor -> retain();
    propertyDescriptor -> typeDescriptor = this;
    _properties[propertyName] = propertyDescriptor;
}

std::list<std::string> LuaExportTypeDescriptor::classMethodNameList()
{
    std::list<std::string> nameList;
    
    for (MethodMap::iterator it = _classMethods.begin(); it != _classMethods.end(); it++)
    {
        nameList.push_back(it -> first);
    }
    
    return nameList;
}

std::list<std::string> LuaExportTypeDescriptor::instanceMethodNameList()
{
    std::list<std::string> nameList;
    
    for (MethodMap::iterator it = _instanceMethods.begin(); it != _instanceMethods.end(); it++)
    {
        nameList.push_back(it -> first);
    }
    
    return nameList;
}

LuaExportMethodDescriptor* LuaExportTypeDescriptor::getClassMethod(std::string methodName, LuaArgumentList arguments)
{
    return filterMethod(methodName, arguments, true);
}

LuaExportMethodDescriptor* LuaExportTypeDescriptor::getInstanceMethod(std::string methodName, LuaArgumentList arguments)
{
    return filterMethod(methodName, arguments, false);
}

LuaExportPropertyDescriptor* LuaExportTypeDescriptor::getProperty(std::string propertyName)
{
    PropertyMap::iterator it = _properties.find(propertyName);
    if (it != _properties.end())
    {
        return it -> second;
    }
    
    return NULL;
}

LuaObjectDescriptor* LuaExportTypeDescriptor::createInstance(LuaSession *session)
{
    return new LuaObjectDescriptor(NULL, this);
}

void LuaExportTypeDescriptor::destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor)
{
    
}

LuaExportTypeDescriptor* LuaExportTypeDescriptor::createSubType(LuaSession *session, std::string subTypeName)
{
    return new LuaExportTypeDescriptor(subTypeName, this);
}

LuaExportMethodDescriptor* LuaExportTypeDescriptor::filterMethod(std::string methodName, LuaArgumentList arguments, bool isStatic)
{
    MethodList methodList;
    if (isStatic)
    {
        MethodMap::iterator mapIt = _classMethods.find(methodName);
        if (mapIt != _classMethods.end())
        {
            methodList = mapIt -> second;
        }
    }
    else
    {
        MethodMap::iterator mapIt = _instanceMethods.find(methodName);
        if (mapIt != _instanceMethods.end())
        {
            methodList = mapIt -> second;
        }
    }

    if (methodList.size() > 1)
    {
        LuaExportMethodDescriptor *targetMethod = NULL;
        
        int startIndex = isStatic ? 0 : 1;
        if (arguments.size() > startIndex)
        {
            //带参数
            std::deque<std::string> signList;
            std::string signListStr;
            std::string signStrRegexp;
            
            for (LuaArgumentList::iterator it = arguments.begin() + startIndex; it != arguments.end(); it++)
            {
                LuaValue *value = *it;
                switch (value -> getType())
                {
                    case LuaValueTypeNumber:
                        signList.push_back("N");
                        signListStr += "N";
                        signStrRegexp += "[fdcislqCISLQB@]";
                        break;
                    case LuaValueTypeBoolean:
                        signList.push_back("B");
                        signListStr += "B";
                        signStrRegexp += "[BcislqCISLQfd@]";
                        break;
                    case LuaValueTypeInteger:
                        signList.push_back("I");
                        signListStr += "I";
                        signStrRegexp += "[cislqCISLQfdB@]";
                        break;
                    default:
                        signList.push_back("O");
                        signListStr += "O";
                        signStrRegexp += "@";
                        break;
                }
            }
            
            std::string luaMethodSignStr = methodName  + "_" + signListStr;
            MappingMethodMap::iterator methodIt = _methodsMapping.find(luaMethodSignStr);
            
            if (methodIt == _methodsMapping.end())
            {
                //映射表无该方法，查找匹配方法
                MethodList matchMethods;
                std::regex regExp(signStrRegexp);
                
                for (MethodList::iterator methodIt = methodList.begin(); methodIt != methodList.end(); methodIt ++)
                {
                    LuaExportMethodDescriptor *methodDesc = *methodIt;
                    if (std::regex_match(methodDesc -> methodSignature(), regExp))
                    {
                        matchMethods.push_back(methodDesc);
                    }
                }
                
                if (matchMethods.size() > 0)
                {
                    //选择最匹配的方法
                    //备选方法，如果没有最匹配的情况下使用
                    LuaExportMethodDescriptor *alternateMethod = NULL;
                    for (MethodList::iterator methodIt = matchMethods.begin(); methodIt != matchMethods.end(); methodIt ++)
                    {
                        LuaExportMethodDescriptor *methodDesc = *methodIt;
                        bool hasMatch = true;
                        bool hasAlternate = false;
                        for (int i = 0; i < methodDesc -> methodSignature().length(); i++)
                        {
                            if (i < signList.size())
                            {
                                std::string luaSign = signList[i];
                                char nativeSign = methodDesc -> methodSignature()[i];
                                if (luaSign == "N" && nativeSign != 'f' && nativeSign != 'd')
                                {
                                    hasAlternate = true;
                                    luaSign = "I";
                                }
                                
                                if (luaSign == "B" && nativeSign != 'B')
                                {
                                    hasMatch = false;
                                    break;
                                }
                                
                                if (luaSign == "I" && nativeSign != 'c' && nativeSign != 'i' && nativeSign != 's' && nativeSign != 'l' && nativeSign != 'q'
                                    && nativeSign != 'C' && nativeSign != 'I' && nativeSign != 'S' && nativeSign != 'L' && nativeSign != 'Q')
                                {
                                    hasMatch = false;
                                    break;
                                }
                                
                                if (luaSign == "O" && nativeSign != '@')
                                {
                                    hasMatch = false;
                                    break;
                                }
                            }
                        }
                        
                        if (hasMatch)
                        {
                            if (hasAlternate)
                            {
                                //记录备选方法
                                alternateMethod = methodDesc;
                            }
                            else
                            {
                                //设置匹配方法
                                targetMethod = methodDesc;
                                break;
                            }
                        }
                    }
                    
                    if (targetMethod == NULL)
                    {
                        if (alternateMethod != NULL)
                        {
                            //使用备选方法
                            targetMethod = alternateMethod;
                        }
                        else
                        {
                            //没有最匹配则使用第一个方法
                            targetMethod = *(matchMethods.begin());
                        }
                    }
                    
                    //设置方法映射
                    _methodsMapping[luaMethodSignStr] = targetMethod;
                }
            }
            else
            {
                targetMethod = methodIt -> second;
            }
        }
        else
        {
            //不带参数
            for (MethodList::iterator it = methodList.begin(); it != methodList.end(); it++)
            {
                LuaExportMethodDescriptor *methodDesc = *it;
                if (methodDesc -> methodSignature() == "")
                {
                    targetMethod = methodDesc;
                    break;
                }
            }
        }
        
        return targetMethod;
    }
    else if (methodList.size() == 1)
    {
        return *(methodList.begin());
    }
    
    return NULL;
}
