//
//  LuaUnityClassImport.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/1.
//  Copyright © 2017年 冯鸿杰. All rights reserved.
//

#include "LuaUnityClassImport.hpp"
#include "LuaUnityExportClassProxy.hpp"
#include "LuaExportClassProxy.h"
#include "LuaObjectEncoder.hpp"
#include "LuaObjectDecoder.hpp"
#include "LuaObjectDescriptor.h"
#include <stdlib.h>

using namespace cn::vimfung::luascriptcore;
using namespace cn::vimfung::luascriptcore::modules::oo;

/**
 * Java对象入栈过滤
 *
 * @param context 上下文对象
 * @param objectDescriptor 需要入栈的对象描述
 */
static bool _unityObjectPushFilter(LuaContext *context, LuaObjectDescriptor *objectDescriptor)
{
    bool filted = false;

    std::string className = objectDescriptor -> getUserdata("NativeClass");
    if (!className.empty())
    {
        filted = LuaClassImport::setLuaMetatable(context, className, objectDescriptor);
    }
    
    return filted;
}

static bool _allowExportsClassHandlerFunc (LuaContext *context, LuaClassImport *classImport, const std::string &className)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    
    LuaAllowExportsClassHandlerPtr allowExportsClassHandlerPtr = unityClassImport -> getAllowExportsClassHandler();
    if (allowExportsClassHandlerPtr != NULL)
    {
        return allowExportsClassHandlerPtr(context -> objectId(), className.c_str());
    }
    
    return false;
}

static LuaExportClassProxy* _exportClassHandlerFunc (LuaContext *context, LuaClassImport *classImport, const std::string &className)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    LuaUnityExportClassProxy *proxy = new LuaUnityExportClassProxy(context,
                                                                   className,
                                                                   unityClassImport -> getAllExportClassMethodHandler(),
                                                                   unityClassImport -> getAllExportInstanceMethodHandler(),
                                                                   unityClassImport -> getAllExportInstanceFieldGetterHandler(),
                                                                   unityClassImport -> getAllExportInstanceFieldSetterHandler());
    return proxy;
}

static LuaValue* _classMethodInvokeHandlerFunc (LuaContext *context,
                                                LuaClassImport *classImport,
                                                LuaObjectDescriptor *classDescriptor,
                                                std::string methodName,
                                                LuaArgumentList args)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    LuaNativeClassMethodInvokeHandlerPtr handler = unityClassImport -> getClassMethodInvokeHandler();
    if (handler != NULL)
    {
        //编码参数列表
        LuaObjectEncoder *encoder = new LuaObjectEncoder(context);
        encoder -> writeInt32((int)args.size());
        
        for (LuaArgumentList::iterator it = args.begin(); it != args.end(); ++it)
        {
            LuaValue *value = *it;
            encoder -> writeObject(value);
        }
        
        //paramsBuffer的内容由C#端进行释放
        const void *paramsBuffer = encoder -> cloneBuffer();
        
        std::string className = (char *)classDescriptor -> getObject();
        void *returnBuffer = handler (context -> objectId(), className.c_str(), methodName.c_str(), paramsBuffer, encoder -> getBufferLength());
        
        encoder -> release();
        
        LuaValue *retValue = NULL;
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, returnBuffer);
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

static LuaValue* _instanceMethodInvokeHandlerFunc (LuaContext *context,
                                                   LuaClassImport *classImport,
                                                   LuaObjectDescriptor *classDescriptor,
                                                   LuaUserdataRef instance,
                                                   std::string methodName,
                                                   LuaArgumentList args)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    LuaNativeInstanceMethodInvokeHandlerPtr handler = unityClassImport -> getInstanceMethodInvokeHandler();
    if (handler != NULL)
    {
        //编码参数列表
        LuaObjectEncoder *encoder = new LuaObjectEncoder(context);
        encoder -> writeInt32((int)args.size());
        
        for (LuaArgumentList::iterator it = args.begin(); it != args.end(); ++it)
        {
            LuaValue *value = *it;
            encoder -> writeObject(value);
        }
        
        LuaObjectDescriptor *objDesc = (LuaObjectDescriptor *)instance -> value;
        std::string className = (char *)classDescriptor -> getObject();
        
        //paramsBuffer的内容由C#端进行释放
        const void *paramsBuffer = encoder -> cloneBuffer();
        void *returnBuffer = handler(context -> objectId(),
                                     className.c_str(),
                                     (long long) objDesc -> getObject(),
                                     methodName.c_str(),
                                     paramsBuffer,
                                     encoder -> getBufferLength());
        
        encoder -> release();
        
        LuaValue *retValue = NULL;
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, returnBuffer);
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

static LuaValue* _instanceFieldGetterInvokeHandlerFunc (LuaContext *context,
                                                        LuaClassImport *classImport,
                                                        LuaObjectDescriptor *classDescriptor,
                                                        LuaUserdataRef instance,
                                                        std::string fieldName)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    
    LuaNativeFieldGetterHandlerPtr methodHandler = unityClassImport -> getFieldGetterHandler();
    if (methodHandler != NULL)
    {
        LuaObjectDescriptor *objDesc = (LuaObjectDescriptor *)instance -> value;
        std::string className = (char *)classDescriptor -> getObject();
        
        void *returnBuffer = methodHandler (context -> objectId(),
                                            className.c_str(),
                                            (long long) objDesc -> getObject(),
                                            fieldName.c_str());
        
        LuaValue *retValue = NULL;
        if (returnBuffer != NULL)
        {
            LuaObjectDecoder *decoder = new LuaObjectDecoder(context, returnBuffer);
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

static void _instanceFieldSetterInvokeHandlerFunc (LuaContext *context,
                                                   LuaClassImport *classImport,
                                                   LuaObjectDescriptor *classDescriptor,
                                                   LuaUserdataRef instance,
                                                   std::string fieldName,
                                                   LuaValue *value)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    
    LuaNativeFieldSetterHandlerPtr methodHandler = unityClassImport -> getFieldSetterHandler();
    if (methodHandler != NULL)
    {
        LuaObjectEncoder *encoder = new LuaObjectEncoder(context);
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
        
        LuaObjectDescriptor *objDesc = (LuaObjectDescriptor *)instance -> value;
        std::string className = (char *)classDescriptor -> getObject();
        
        methodHandler (context -> objectId(),
                       className.c_str(),
                       (long long) objDesc -> getObject(),
                       fieldName.c_str(),
                       valueBuf,
                       encoder -> getBufferLength());
        
        encoder -> release();
    }
}

static LuaObjectDescriptor* _createInstanceHandlerFunc (LuaContext *context,
                                                        LuaClassImport *classImport,
                                                        LuaObjectDescriptor *classDescriptor)
{
    LuaUnityClassImport *unityClassImport = (LuaUnityClassImport *)classImport;
    
    LuaCreateNativeObjectHandlerPtr methodHandler = unityClassImport -> getCreateObjectHandler();
    if (methodHandler != NULL)
    {
        std::string className = (char *)classDescriptor -> getObject();
        long long instance = methodHandler (context -> objectId(), className.c_str());
        LuaObjectDescriptor *objDesc = new LuaObjectDescriptor((void *)instance);
        return objDesc;
    }
    
    return NULL;
}

LuaUnityClassImport::LuaUnityClassImport()
{
    onAllowExportsClass(_allowExportsClassHandlerFunc);
    onExportsClass(_exportClassHandlerFunc);
    onClassMethodInvoke(_classMethodInvokeHandlerFunc);
    onInstanceMethodInvoke(_instanceMethodInvokeHandlerFunc);
    onInstanceFieldGetterInvoke(_instanceFieldGetterInvokeHandlerFunc);
    onInstanceFieldSetterInvoke(_instanceFieldSetterInvokeHandlerFunc);
    onCreateInstance(_createInstanceHandlerFunc);
}

void LuaUnityClassImport::onRegister(const std::string &name, LuaContext *context)
{
    //添加对象过滤器
    LuaObjectDescriptor::addPushFilter(_unityObjectPushFilter);
    LuaClassImport::onRegister(name, context);
}

void LuaUnityClassImport::setAllowExportsClassHandler(LuaAllowExportsClassHandlerPtr handler)
{
    _allowExportsClassHandlerPtr = handler;
}

void LuaUnityClassImport::setAllExportClassMethodHandler(LuaAllExportClassMethodHandlerPtr handler)
{
    _allExportClassMethodHandlerPtr = handler;
}

void LuaUnityClassImport::setAllExportInstanceMethodHandler(LuaAllExportInstanceMethodHandlerPtr handler)
{
    _allExportInstanceMethodHandlerPtr = handler;
}

void LuaUnityClassImport::setAllExportInstanceFieldGetterHandler(LuaAllExportFieldGetterHandlerPtr handler)
{
    _allExportInstanceFieldGetterHandlerPtr = handler;
}

void LuaUnityClassImport::setAllExportInstanceFieldSetterHandler(LuaAllExportFieldSetterHandlerPtr handler)
{
    _allExportInstanceFieldSetterHandlerPtr = handler;
}

void LuaUnityClassImport::setCreateObjectHandler(LuaCreateNativeObjectHandlerPtr handler)
{
    _createObjectHandlerPtr = handler;
}

void LuaUnityClassImport::setClassMethodInvokeHandler(LuaNativeClassMethodInvokeHandlerPtr handler)
{
    _classMethodInvokeHandlerPtr = handler;
}

void LuaUnityClassImport::setInstanceMethodInvokeHandler(LuaNativeInstanceMethodInvokeHandlerPtr handler)
{
    _instanceMethodInvokeHandlerPtr = handler;
}

void LuaUnityClassImport::setFieldGetterHandler(LuaNativeFieldGetterHandlerPtr handler)
{
    _fieldGetterHandlerPtr = handler;
}

void LuaUnityClassImport::setFieldSetterHandler(LuaNativeFieldSetterHandlerPtr handler)
{
    _fieldSetterHandlerPtr = handler;
}

LuaAllowExportsClassHandlerPtr LuaUnityClassImport::getAllowExportsClassHandler()
{
    return _allowExportsClassHandlerPtr;
}

LuaAllExportClassMethodHandlerPtr LuaUnityClassImport::getAllExportClassMethodHandler()
{
    return _allExportClassMethodHandlerPtr;
}

LuaAllExportInstanceMethodHandlerPtr LuaUnityClassImport::getAllExportInstanceMethodHandler()
{
    return _allExportInstanceMethodHandlerPtr;
}

LuaAllExportFieldGetterHandlerPtr LuaUnityClassImport::getAllExportInstanceFieldGetterHandler()
{
    return _allExportInstanceFieldGetterHandlerPtr;
}

LuaAllExportFieldSetterHandlerPtr LuaUnityClassImport::getAllExportInstanceFieldSetterHandler()
{
    return _allExportInstanceFieldSetterHandlerPtr;
}

LuaCreateNativeObjectHandlerPtr LuaUnityClassImport::getCreateObjectHandler()
{
    return _createObjectHandlerPtr;
}

LuaNativeClassMethodInvokeHandlerPtr LuaUnityClassImport::getClassMethodInvokeHandler()
{
    return _classMethodInvokeHandlerPtr;
}

LuaNativeInstanceMethodInvokeHandlerPtr LuaUnityClassImport::getInstanceMethodInvokeHandler()
{
    return _instanceMethodInvokeHandlerPtr;
}

LuaNativeFieldGetterHandlerPtr LuaUnityClassImport::getFieldGetterHandler()
{
    return _fieldGetterHandlerPtr;
}

LuaNativeFieldSetterHandlerPtr LuaUnityClassImport::getFieldSetterHandler()
{
    return _fieldSetterHandlerPtr;
}
