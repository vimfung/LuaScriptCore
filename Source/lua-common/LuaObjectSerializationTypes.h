//
//  LuaObjectSerializationTypes.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/17.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef LuaObjectSerializationTypes_h
#define LuaObjectSerializationTypes_h

typedef union
{
    double value;
    unsigned char   bytes[8];
    
}DoubleStruct;

#endif /* LuaObjectSerializationTypes_h */
