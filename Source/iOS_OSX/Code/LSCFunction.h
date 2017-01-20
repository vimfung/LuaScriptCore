//
//  LSCFunction.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCValue;


/**
 Lua方法
 */
@interface LSCFunction : NSObject

/**
 调用方法
 
 @param arguments 参数列表
 
 @return 返回值, 如果返回值数量为1，则返回类型为LSCValue， 如果>1则返回类型为LSCTuple
 */
- (id)invokeWithArguments:(NSArray<LSCValue *> *)arguments;

@end
