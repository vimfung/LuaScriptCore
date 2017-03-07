//
//  LuaObjectDecoder.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#include "LuaObjectDecoder.hpp"
#include "LuaObjectSerializationTypes.h"
#include "LuaNativeClass.hpp"
#include "LuaObjectDescriptor.h"
#include "LuaContext.h"
#include "LuaObjectManager.h"
#include <memory.h>

using namespace cn::vimfung::luascriptcore;

LuaObjectDecoder::LuaObjectDecoder(LuaContext *context, const void *buf)
    :_buf(buf), _offset(0), _context(context)
{
    
}

LuaObjectDecoder::~LuaObjectDecoder()
{
    _buf = NULL;
    _offset = 0;
}

LuaContext* LuaObjectDecoder::getContext()
{
    return _context;
}

char LuaObjectDecoder::readByte()
{
    char value = ((char *)_buf) [_offset];
    _offset++;
    
    return value;
}

short LuaObjectDecoder::readInt16()
{
    short value = (((unsigned char *)_buf) [_offset] << 8)
				| ((unsigned char *)_buf) [_offset + 1];
    _offset += 2;
    
    return value;
}

int LuaObjectDecoder::readInt32()
{
    int value = (((unsigned char *)_buf) [_offset] << 24)
				| (((unsigned char *)_buf) [_offset + 1] << 16)
				| (((unsigned char *)_buf) [_offset + 2] << 8)
				| (((unsigned char *)_buf) [_offset + 3]);
    _offset += 4;
    
    return value;
}

long long LuaObjectDecoder::readInt64()
{
    long long value = ((long long)((unsigned char *)_buf) [_offset] << 56)
				| ((long long)((unsigned char *)_buf) [_offset + 1] << 48)
				| ((long long)((unsigned char *)_buf) [_offset + 2] << 40)
				| ((long long)((unsigned char *)_buf) [_offset + 3] << 32)
				| (((unsigned char *)_buf) [_offset + 4] << 24)
				| (((unsigned char *)_buf) [_offset + 5] << 16)
				| (((unsigned char *)_buf) [_offset + 6] << 8)
				| ((unsigned char *)_buf) [_offset + 7];
    _offset += 8;
    
    return value;
}

double LuaObjectDecoder::readDouble()
{
    DoubleStruct ds;
    memcpy(ds.bytes, (((unsigned char *)_buf) + _offset), 8);
    _offset += 8;
    
    return ds.value;
}

const std::string LuaObjectDecoder::readString()
{
    std::string str;
    
    int size = readInt32();
    char *buf = new char[size + 1] {0};
    memcpy(buf, (char *)_buf + _offset, size);
    _offset += size;
    str = buf;
    delete[] buf;
    
    return str;
}

void LuaObjectDecoder::readBytes(void **bytes, int *length)
{
    *length = readInt32();
    
    *bytes = new char[*length];
    memcpy(*bytes, (char *)_buf + _offset, *length);
    _offset += *length;
}

LuaObject* LuaObjectDecoder::readObject()
{
    if (((char *)_buf) [_offset] == 'L')
    {
        _offset ++;
        
        std::string className = readString ();
        
        if (readByte () == ';')
        {
            LuaNativeClass *nativeClass = LuaNativeClass::findClass(className);
            if (nativeClass != NULL)
            {
                //取出对象标识，并从对象管理器中查找是否存在此对象。
                int objectId = readInt32();
                LuaObject *obj = LuaObject::findObject(objectId);
                if (obj == NULL)
                {
                    //恢复读取对象标识的游标
                    _offset -= 4;
                    obj = (LuaObject *)nativeClass -> createInstance(this);
                }
                else
                {
                    //增加一次引用
                    obj -> retain();
                }
                
                return obj;
            }
        }
    }
    else
    {
        //其他原生类型使用ObjectDescriptor装载
        void **objRef = NULL;
        objRef = (void **)readInt64();
        
        LuaObjectDescriptor *objDesc = new LuaObjectDescriptor(*objRef);
        return objDesc;
    }
    
    return NULL;
}
