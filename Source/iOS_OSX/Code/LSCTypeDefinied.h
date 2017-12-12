//
//  LSCTypeDefinied.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#ifndef LSCTypeDefinied_h
#define LSCTypeDefinied_h

@class LSCValue;

typedef NS_ENUM(NSUInteger, LSCValueType)
{
    LSCValueTypeNil = 0,
    LSCValueTypeNumber = 1,
    LSCValueTypeBoolean = 2,
    LSCValueTypeString = 3,
    LSCValueTypeArray = 4,
    LSCValueTypeMap = 5,
    LSCValueTypePtr = 6,
    LSCValueTypeObject = 7,
    LSCValueTypeInteger = 8,
    LSCValueTypeData = 9,
    LSCValueTypeFunction = 10,
    LSCValueTypeTuple = 11,
    LSCValueTypeClass = 12
};

/**
 *  异常处理器
 *
 *  @param message 异常信息
 */
typedef void (^LSCExceptionHandler) (NSString *message);

/**
 *  方法处理器
 *
 *  @param arguments 参数列表
 *
 *  @return 返回值
 */
typedef LSCValue* (^LSCFunctionHandler) (NSArray<LSCValue *> *arguments);


#endif /* LSCTypeDefinied_h */
