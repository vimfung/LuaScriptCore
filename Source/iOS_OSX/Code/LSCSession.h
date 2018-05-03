//
//  LSCCallSession.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/7/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCValue;

/**
 会话，从Lua调用原生方法时依靠此会话来处理参数和返回值
 */
@interface LSCSession : NSObject

/**
 前一个会话
 */
@property (nonatomic, weak) LSCSession *prevSession;

/**
 解析并获取参数

 @return 参数集合
 */
- (NSArray *)parseArguments;


/**
 解析并获取参数（排除第一个参数）

 @return 参数集合
 */
- (NSArray *)parseArgumentsWithoutTheFirst;

/**
 设置返回值

 @param value 返回值
 @returns 参数数量
 */
- (int)setReturnValue:(LSCValue *)value;

@end
