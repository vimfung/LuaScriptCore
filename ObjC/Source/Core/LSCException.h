//
//  LSCException.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

#define LSC_EXCEPTION_NAME @"LuaScriptCore"

#define CANNOT_INIT_OBJ_EXCEPTION [NSException exceptionWithName:LSC_EXCEPTION_NAME reason:@"Cannot use the init method to construct an instance" userInfo:nil]

#define SCRIPT_CANNOT_BE_NIL_EXCEPTION [NSException exceptionWithName:LSC_EXCEPTION_NAME reason:@"Script cannot be nil" userInfo:nil]

#define SCRIPT_PATH_INVALID_EXCEPTION [NSException exceptionWithName:LSC_EXCEPTION_NAME reason:@"Lua file path is invalid" userInfo:nil]

#define INVALID_PARAMS_EXCEPTION [NSException exceptionWithName:LSC_EXCEPTION_NAME reason:@"Invalid paramters" userInfo:nil]

#define TUPLE_CANNOT_BE_SET_EXCEPTION [NSException exceptionWithName:LSC_EXCEPTION_NAME reason:@"Tuple cannot be set as a global variable" userInfo:nil];

@class LSCContext;

/**
 *  异常处理器
 *
 *  @param message 异常信息
 */
typedef void (^LSCExceptionHandler) (LSCContext *context, NSString *message);
