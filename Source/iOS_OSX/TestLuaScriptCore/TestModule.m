//
//  TestModule.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "TestModule.h"
#import "LSCModule_Private.h"

@implementation TestModule

- (instancetype)init
{
    if (self = [super init])
    {
        self.version = @"1.0.0";
        self.desc = @"这是一个测试模块";
    }
    return self;
}

- (void)test
{
    NSLog(@"Hello World!");
}

- (int)callWithName:(NSString *)name byIndex:(NSInteger)index
{
    NSLog(@"%@ at %d", name, index);
    
    return 1024;
}

@end
