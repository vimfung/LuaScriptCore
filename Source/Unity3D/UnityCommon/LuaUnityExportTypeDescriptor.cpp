//
//  LuaUnityExportTypeDescriptor.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/27.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityExportTypeDescriptor.hpp"
#include "LuaUnityEnv.hpp"
#include "LuaObjectDescriptor.h"
#include "LuaSession.h"
#include "LuaValue.h"
#include "LuaEngineAdapter.hpp"
#include "LuaObjectEncoder.hpp"

LuaUnityExportTypeDescriptor::LuaUnityExportTypeDescriptor(std::string const& name, LuaExportTypeDescriptor *parentTypeDescriptor)
    : LuaExportTypeDescriptor(name, parentTypeDescriptor)
{
    
}

LuaExportTypeDescriptor* LuaUnityExportTypeDescriptor::createSubType(LuaSession *session, std::string const& subTypeName)
{
    LuaUnityExportTypeDescriptor *subTypeDescriptor = new LuaUnityExportTypeDescriptor(subTypeName, this);
    
    subTypeDescriptor -> createInstanceHandler = createInstanceHandler;
    subTypeDescriptor -> destroyInstanceHandler = destroyInstanceHandler;
    subTypeDescriptor -> instanceDescriptionHandler = instanceDescriptionHandler;
    
    return subTypeDescriptor;
}

LuaObjectDescriptor* LuaUnityExportTypeDescriptor::createInstance(LuaSession *session)
{
    LuaObjectDescriptor *objectDescriptor = NULL;
    
    if (createInstanceHandler != NULL)
    {
        LuaContext *context = session -> getContext();
        
        //解析参数
        LuaArgumentList arguments;
        session -> parseArguments(arguments, 2);
        
        //参数编码
        LuaObjectEncoder *encoder = new LuaObjectEncoder(context);
        encoder -> writeInt32((int)arguments.size());
        
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
        {
            LuaValue *value = *it;
            encoder -> writeObject(value);
        }
        
        const void *paramsBuffer = encoder -> cloneBuffer();
        long long instanceId = createInstanceHandler(context -> objectId(), objectId(), paramsBuffer, encoder -> getBufferLength());
        encoder -> release();
        
        if (instanceId != -1)
        {
            objectDescriptor = new LuaObjectDescriptor(context, (void *)instanceId, this);
            
            //设置本地对象标识
            LuaUnityEnv::sharedInstance() -> setNativeObjectId(objectDescriptor -> getObject(),
                                                               objectDescriptor -> objectId(),
                                                               objectDescriptor -> getExchangeId());
        }
        else
        {
            session -> reportLuaException("Unsupported constructor method");
        }
        
        
        //释放参数内存
        for (LuaArgumentList::iterator it = arguments.begin(); it != arguments.end(); ++it)
        {
            LuaValue *value = *it;
            value -> release();
        }
    }
    
    return objectDescriptor;
}

void LuaUnityExportTypeDescriptor::destroyInstance(LuaSession *session, LuaObjectDescriptor *objectDescriptor)
{
    if (destroyInstanceHandler != NULL)
    {
        destroyInstanceHandler((long long)objectDescriptor -> getObject());
    }
}
