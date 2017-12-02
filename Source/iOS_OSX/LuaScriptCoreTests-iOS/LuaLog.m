//
//  LuaLog.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/6.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LuaLog.h"

@implementation LuaLog

+ (void)writeLog:(NSString *)msg
{
    NSLog(@"%@ log = %@", self, msg);
}

@end
