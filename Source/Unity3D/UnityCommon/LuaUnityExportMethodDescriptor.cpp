//
//  LuaUnityExportClassMethodDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/28.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityExportMethodDescriptor.hpp"
#include "LuaValue.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaExportTypeDescriptor.hpp"
#include "LuaObjectDescriptor.h"
#include "StringUtils.h"
#include <stdlib.h>

LuaUnityExportMethodDescriptor::LuaUnityExportMethodDescriptor(std::string const& name, std::string const& methodSignature, LuaModuleMethodHandlerPtr handler)
    : LuaExportMethodDescriptor(name, methodSignature)
{
    _methodType = LuaUnityExportMethodTypeClass;
    _classMethodHandler = handler;
}

LuaUnityExportMethodDescriptor::LuaUnityExportMethodDescriptor(std::string const& name, std::string const& methodSignature, LuaInstanceMethodHandlerPtr handler)
    : LuaExportMethodDescriptor(name, methodSignature)
{
    _methodType = LuaUnityExportMethodTypeInstance;
    _instanceMethodHandler = handler;
}

LuaValue* LuaUnityExportMethodDescriptor::invoke(LuaSession *session, LuaArgumentList arguments)
{
    switch (_methodType)
    {
        case LuaUnityExportMethodTypeClass:
            return invokeClassMethod(session, arguments);
        case LuaUnityExportMethodTypeInstance:
            return invokeInstanceMethod(session, arguments);
        default:
            return NULL;
    }
}

LuaValue* LuaUnityExportMethodDescriptor::invokeClassMethod(LuaSession *session, LuaArgumentList arguments)
{
    if (_classMethodHandler != NULL)
    {
        //编码参数列表
        LuaObjectEncoder *encoder = new LuaObjectEncoder(session -> getContext());
        encoder -> writeInt32((int)arguments.size());
        
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
        {
            LuaValue *value = *it;
            encoder -> writeObject(value);
        }
        
        //paramsBuffer的内容由C#端进行释放
        std::string methodName = StringUtils::format("%s_%s", name().c_str(), methodSignature().c_str());
        const void *paramsBuffer = encoder -> cloneBuffer();
        void *returnBuffer = _classMethodHandler(session -> getContext() -> objectId(),
                                                 typeDescriptor -> objectId(),
                                                 methodName.c_str(),
                                                 paramsBuffer,
                                                 encoder -> getBufferLength());

        encoder -> release();

        LuaValue *retValue = NULL;
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(session -> getContext(), returnBuffer);
            retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
            decoder -> release();

            //释放C＃中申请的内存
            free(returnBuffer);
        }
        else
        {
            retValue = LuaValue::NilValue();
        }
        
        return retValue;
    }
    
    return NULL;
}

LuaValue* LuaUnityExportMethodDescriptor::invokeInstanceMethod(LuaSession *session, LuaArgumentList arguments)
{
    if (_instanceMethodHandler != NULL)
    {
        //编码参数列表
        LuaObjectEncoder *encoder = new LuaObjectEncoder(session -> getContext());
        encoder -> writeInt32((int)(arguments.size() - 1));
        
        LuaArgumentList::iterator it = arguments.begin();
        LuaObjectDescriptor *instance = (*it) -> toObject();
        ++it;
        
        for (; it != arguments.end(); ++it)
        {
            LuaValue *value = *it;
            encoder -> writeObject(value);
        }
        
        //paramsBuffer的内容由C#端进行释放
        std::string methodName = StringUtils::format("%s_%s", name().c_str(), methodSignature().c_str());
        const void *paramsBuffer = encoder -> cloneBuffer();
        void *returnBuffer = _instanceMethodHandler(session -> getContext() -> objectId(),
                                                    typeDescriptor -> objectId(),
                                                    (long long) instance -> getObject(),
                                                    methodName.c_str(),
                                                    paramsBuffer,
                                                    encoder -> getBufferLength());
        
        encoder -> release();
        
        LuaValue *retValue = NULL;
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(session -> getContext(), returnBuffer);
            retValue = dynamic_cast<LuaValue *>(decoder -> readObject());
            decoder -> release();
            
            //释放C＃中申请的内存
            free(returnBuffer);
        }
        else
        {
            retValue = LuaValue::NilValue();
        }
        
        return retValue;
    }
    
    return NULL;
}
