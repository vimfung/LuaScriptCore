//
//  LogModule.h
//  Sample
//
//  Created by 冯鸿杰 on 16/9/22.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"

/**
 *  日志模块
 */
@interface LogModule : LSCModule

/**
 *  写入日志
 *
 *  @param message 日志信息
 */
+ (void)writeLog:(NSString *)message;

@end
