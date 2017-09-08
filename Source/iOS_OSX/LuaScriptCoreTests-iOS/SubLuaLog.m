//
//  SubLuaLog.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "SubLuaLog.h"

@implementation SubLuaLog

+ (NSString *)typeName
{
    return @"ChildLog";
}

- (void)printName
{
    NSLog(@"%@ name = %@", self, self.name);
}

@end
