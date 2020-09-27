//
//  LuaChildType.m
//  LuaScriptCoreTests
//
//  Created by 冯鸿杰 on 2020/9/22.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LuaChildType.h"

@implementation LuaChildType

- (void)setA:(NSString *)A
{
    NSLog(@"--------- child set a");
    [super setA:A];
}

- (void)setBStr:(NSString * _Nonnull)bStr
{
    NSLog(@"----- cannot set b str");
}

- (NSString *)dStr
{
    return @"Hello";
}

- (void)setDStr:(NSString *)dStr
{
    NSLog(@"------ set c str");
}

@end
