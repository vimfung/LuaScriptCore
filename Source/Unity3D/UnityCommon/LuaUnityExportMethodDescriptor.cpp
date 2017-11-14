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

LuaUnityExportMethodDescriptor::LuaUnityExportMethodDescriptor(std::string name, std::string methodSignature, LuaModuleMethodHandlerPtr handler) : LuaExportMethodDescriptor(name, methodSignature)
{
    _methodType = LuaUnityExportMethodTypeClass;
    _classMethodHandler = handler;
}

LuaUnityExportMethodDescriptor::LuaUnityExportMethodDescriptor(std::string name, std::string methodSignature, LuaInstanceMethodHandlerPtr handler) : LuaExportMethodDescriptor(name, methodSignature)
{
    _methodType = LuaUnityExportMethodTypeInstance;
    _instanceMethodHandler = handler;
}

LuaUnityExportMethodDescriptor::LuaUnityExportMethodDescriptor(std::string name, std::string methodSignature, std::string fieldName, LuaInstanceFieldGetterHandlerPtr handler) : LuaExportMethodDescriptor(name, methodSignature)
{
    _methodType = LuaUnityExportMethodTypeGetter;
    _fieldGetterMethodHandler = handler;
    _fieldName = fieldName;
}

LuaUnityExportMethodDescriptor::LuaUnityExportMethodDescriptor(std::string name, std::string methodSignature, std::string fieldName, LuaInstanceFieldSetterHandlerPtr handler) : LuaExportMethodDescriptor(name, methodSignature)
{
    _methodType = LuaUnityExportMethodTypeSetter;
    _fieldSetterMethodHandler = handler;
    _fieldName = fieldName;
}

LuaValue* LuaUnityExportMethodDescriptor::invoke(LuaSession *session, LuaArgumentList arguments)
{
    switch (_methodType)
    {
        case LuaUnityExportMethodTypeClass:
            return invokeClassMethod(session, arguments);
        case LuaUnityExportMethodTypeInstance:
            return invokeInstanceMethod(session, arguments);
        case LuaUnityExportMethodTypeGetter:
            return invokeGetterMethod(session, arguments);
        case LuaUnityExportMethodTypeSetter:
            return invokeSetterMethod(session, arguments);
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
        void *returnBuffer = _classMethodHandler(typeDescriptor -> objectId(), methodName.c_str(), paramsBuffer, encoder -> getBufferLength());

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
        void *returnBuffer = _instanceMethodHandler(typeDescriptor -> objectId(),
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

LuaValue* LuaUnityExportMethodDescriptor::invokeGetterMethod(LuaSession *session, LuaArgumentList arguments)
{
    if (_fieldGetterMethodHandler != NULL)
    {
        LuaArgumentList::iterator it = arguments.begin();
        LuaObjectDescriptor *instance = (*it) -> toObject();
        
        void *returnBuffer = _fieldGetterMethodHandler (typeDescriptor -> objectId(), (long long) instance -> getObject(), _fieldName.c_str());
        
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

LuaValue* LuaUnityExportMethodDescriptor::invokeSetterMethod(LuaSession *session, LuaArgumentList arguments)
{
    if (_fieldSetterMethodHandler != NULL)
    {
        LuaArgumentList::iterator it = arguments.begin();
        LuaObjectDescriptor *instance = (*it) -> toObject();
        ++it;
        
        LuaValue *value = *it;
        LuaObjectEncoder *encoder = new LuaObjectEncoder(session -> getContext());
        if (value != NULL)
        {
            encoder -> writeObject(value);
        }
        else
        {
            LuaValue *nilValue = LuaValue::NilValue();
            encoder -> writeObject(nilValue);
            nilValue -> release();
        }
        
        //valueBuf的内容由C#端进行释放
        const void *valueBuf = encoder -> cloneBuffer();
        
        _fieldSetterMethodHandler (typeDescriptor -> objectId(),
                                   (long long) instance -> getObject(),
                                   _fieldName.c_str(),
                                   valueBuf,
                                   encoder -> getBufferLength());
        
        encoder -> release();
    }
    
    return NULL;
}
