//
//  LuaUnityExportPropertyDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/30.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityExportPropertyDescriptor.hpp"
#include "LuaObjectDescriptor.h"
#include "LuaValue.h"
#include "LuaExportTypeDescriptor.hpp"
#include "LuaSession.h"
#include "LuaObjectDecoder.hpp"
#include "LuaObjectEncoder.hpp"

LuaUnityExportPropertyDescriptor::LuaUnityExportPropertyDescriptor(std::string name,
                                                                   bool canRead,
                                                                   bool canWrite,
                                                                   LuaInstanceFieldGetterHandlerPtr getterHandler,
                                                                   LuaInstanceFieldSetterHandlerPtr setterHandler)
        :LuaExportPropertyDescriptor(name, canRead, canWrite)
{
    _getterHandler = getterHandler;
    _setterHandler = setterHandler;
}

LuaValue* LuaUnityExportPropertyDescriptor::invokeGetter(LuaSession *session, LuaObjectDescriptor *instance)
{
    if (_getterHandler != NULL && canRead())
    {
        void *returnBuffer = _getterHandler (typeDescriptor -> objectId(), (long long) instance -> getObject(), name().c_str());
        
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

void LuaUnityExportPropertyDescriptor::invokeSetter(LuaSession *session, LuaObjectDescriptor *instance, LuaValue *value)
{
    if (_setterHandler != NULL && canWrite())
    {
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
        
        _setterHandler (typeDescriptor -> objectId(),
                        (long long) instance -> getObject(),
                        name().c_str(),
                        valueBuf,
                        encoder -> getBufferLength());
        
        encoder -> release();
    }
}
