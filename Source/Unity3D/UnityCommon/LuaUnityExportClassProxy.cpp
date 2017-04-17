//
//  LuaUnityExportClassProxy.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/5.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityExportClassProxy.hpp"
#include "LuaObjectDescriptor.h"
#include "LuaObjectDecoder.hpp"
#include "LuaUnityClassObjectDescriptor.hpp"
#include <stdlib.h>

using namespace cn::vimfung::luascriptcore;
using namespace cn::vimfung::luascriptcore::modules::oo;

LuaUnityExportClassProxy::LuaUnityExportClassProxy(LuaContext *context,
                         std::string className,
                         LuaAllExportClassMethodHandlerPtr allExportClassMethodHandlerPtr,
                         LuaAllExportInstanceMethodHandlerPtr allExportInstanceMethodHandlerPtr,
                         LuaAllExportFieldGetterHandlerPtr allExportInstanceFieldGetterHandlerPtr,
                         LuaAllExportFieldSetterHandlerPtr allExportInstanceFieldSetterHandlerPtr)
    :_context(context), _allExportClassMethodHandlerPtr(allExportClassMethodHandlerPtr), _allExportInstanceMethodHandlerPtr(allExportInstanceMethodHandlerPtr), _allExportInstanceFieldGetterHandlerPtr(allExportInstanceFieldGetterHandlerPtr), _allExportInstanceFieldSetterHandlerPtr(allExportInstanceFieldSetterHandlerPtr)
{
    _classDescriptor = new LuaUnityClassObjectDescriptor(className);
}

LuaUnityExportClassProxy::~LuaUnityExportClassProxy()
{
    _classDescriptor -> release();
}

LuaObjectDescriptor* LuaUnityExportClassProxy::getExportClass()
{
    return _classDescriptor;
}

LuaExportNameList LuaUnityExportClassProxy::allExportClassMethods()
{
    LuaExportNameList nameList;
    
    if (_allExportClassMethodHandlerPtr != NULL)
    {
        std::string className = (char *)_classDescriptor -> getObject();
        void *returnBuffer = _allExportClassMethodHandlerPtr(_context -> objectId(), className.c_str());
        
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(_context, returnBuffer);
            
            int size = decoder -> readInt32();
            for (int i = 0; i < size; i++)
            {
                std::string name = decoder -> readString();
                nameList.push_back(name);
            }
            
            decoder -> release();
            
            //释放C＃中申请的内存
            free(returnBuffer);
        }
    }
    
    return nameList;
}

LuaExportNameList LuaUnityExportClassProxy::allExportInstanceMethods()
{
    LuaExportNameList nameList;
    
    if (_allExportInstanceMethodHandlerPtr != NULL)
    {
        std::string className = (char *)_classDescriptor -> getObject();
        void *returnBuffer = _allExportInstanceMethodHandlerPtr(_context -> objectId(), className.c_str());
        
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(_context, returnBuffer);
            
            int size = decoder -> readInt32();
            for (int i = 0; i < size; i++)
            {
                std::string name = decoder -> readString();
                nameList.push_back(name);
            }
            
            decoder -> release();
            
            //释放C＃中申请的内存
            free(returnBuffer);
        }
    }
    
    return nameList;
}

LuaExportNameList LuaUnityExportClassProxy::allExportGetterFields()
{
    LuaExportNameList nameList;
    
    if (_allExportInstanceFieldGetterHandlerPtr != NULL)
    {
        std::string className = (char *)_classDescriptor -> getObject();
        void *returnBuffer = _allExportInstanceFieldGetterHandlerPtr(_context -> objectId(), className.c_str());
        
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(_context, returnBuffer);
            
            int size = decoder -> readInt32();
            for (int i = 0; i < size; i++)
            {
                std::string name = decoder -> readString();
                nameList.push_back(name);
            }
            
            decoder -> release();
            
            //释放C＃中申请的内存
            free(returnBuffer);
        }
    }
    
    return nameList;
}

LuaExportNameList LuaUnityExportClassProxy::allExportSetterFields()
{
    LuaExportNameList nameList;
    
    if (_allExportInstanceFieldSetterHandlerPtr != NULL)
    {
        std::string className = (char *)_classDescriptor -> getObject();
        void *returnBuffer = _allExportInstanceFieldSetterHandlerPtr(_context -> objectId(), className.c_str());
        
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(_context, returnBuffer);
            
            int size = decoder -> readInt32();
            for (int i = 0; i < size; i++)
            {
                std::string name = decoder -> readString();
                nameList.push_back(name);
            }
            
            decoder -> release();
            
            //释放C＃中申请的内存
            free(returnBuffer);
        }
    }
    
    return nameList;
}
