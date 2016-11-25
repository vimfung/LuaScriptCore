//
//  LuaObjectEncoder.cpp
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/15.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#include "LuaObjectEncoder.hpp"
#include "LuaObjectSerializationTypes.h"
#include <stdlib.h>
#include <memory.h>

using namespace cn::vimfung::luascriptcore;

LuaObjectEncoder::LuaObjectEncoder ()
    :_buf(NULL), _bufLength(0)
{
    
}

LuaObjectEncoder::~LuaObjectEncoder()
{
    if (_buf != NULL)
    {
        free(_buf);
        _buf = NULL;
    }
}

void LuaObjectEncoder::reallocBuffer(int size)
{
    if (_bufLength == 0)
    {
        _bufLength = size;
        _buf = malloc(size);
        memset(_buf, 0, _bufLength);
    }
    else
    {
        _buf = realloc(_buf, _bufLength + size);
        
        void *offset = (char *)_buf + _bufLength;
        memset(offset, 0, size);
        _bufLength += size;
    }
}

void LuaObjectEncoder::writeByte(char value)
{
    writeBuffer(&value, 1);
}

void LuaObjectEncoder::writeBuffer(const void *bytes, int length)
{
    reallocBuffer(length);
    
    void *offset = (void *)((char *)_buf + _bufLength - length);
    memcpy(offset, bytes, length);
}

void LuaObjectEncoder::writeInt16(short value)
{
    char buf[2] = {0};
    buf[0] = value >> 8 & 0xff;
    buf[1] = value & 0xff;
    
    writeBuffer(buf, 2);
}

void LuaObjectEncoder::writeInt32(int value)
{
    char buf[4] = {0};
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;
    
    writeBuffer(buf, 4);
}

void LuaObjectEncoder::writeInt64(long long value)
{
    char buf[8] = {0};
    buf[0] = value >> 56 & 0xff;
    buf[1] = value >> 48 & 0xff;
    buf[2] = value >> 40 & 0xff;
    buf[3] = value >> 32 & 0xff;
    buf[4] = value >> 24 & 0xff;
    buf[5] = value >> 16 & 0xff;
    buf[6] = value >> 8 & 0xff;
    buf[7] = value & 0xff;
    
    writeBuffer(buf, 8);
}

void LuaObjectEncoder::writeDouble(double value)
{
    DoubleStruct ds;
    ds.value = value;
    writeBuffer(ds.bytes, 8);
}

void LuaObjectEncoder::writeString(const std::string &value)
{
    const char *cstrValue = value.c_str();
    size_t length = strlen(cstrValue);
    writeInt32((int)length);
    writeBuffer(cstrValue, (int)length);
}

void LuaObjectEncoder::writeObject(LuaObject *object)
{
    object -> serialization("", this);
}

const void* LuaObjectEncoder::getBuffer()
{
    return _buf;
}

const void* LuaObjectEncoder::cloneBuffer()
{
    void *cloneBuf = malloc(_bufLength * sizeof(char));
    memcpy(cloneBuf, _buf, _bufLength);
    return cloneBuf;
}

int LuaObjectEncoder::getBufferLength()
{
    return _bufLength;
}

int LuaObjectEncoder::encodeObject(LuaObject *object, const void** bytes)
{
    LuaObjectEncoder *encoder = new LuaObjectEncoder();
    encoder -> writeObject(object);
    *bytes = (void *)encoder -> cloneBuffer();
    
    int bufferLen = encoder -> getBufferLength();
    encoder -> release();
    
    return bufferLen;
}
