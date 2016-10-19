//
//  LogModule.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LogModule.h"

@implementation LogModule

+ (NSString *)version
{
    return @"1.0.0";
}

+ (void)writeLog:(NSString *)m
{
    NSLog(@"%@", m);
}

@end
