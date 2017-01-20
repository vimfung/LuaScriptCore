//
//  TestModule.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "TestModule.h"
#import "LSCTuple.h"

@implementation TestModule

+ (NSString *)test
{
    return @"Hello World!";
}

+ (LSCTuple *)testTuple
{
    LSCTuple *tuple = [[LSCTuple alloc] init];
    [tuple addReturnValue:@"Hello"];
    [tuple addReturnValue:@"World"];
    
    return tuple;
}


@end
