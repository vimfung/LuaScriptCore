//
//  ViewController.m
//  TestLuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "ViewController.h"
#import "LuaScriptCore.h"
#import "TestModule.h"
#import "LSCObjectClass.h"

#import <objc/runtime.h>

@interface Base : NSObject

- (void)test;

@end

@implementation Base

//- (void)test
//{
//    NSLog(@"Hello World");
//}

@end

@interface Sub : Base

@end

@implementation Sub

- (void)test
{
    NSLog(@"sub");
}

@end

@interface ViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    
//    IMP baseIMP = class_getMethodImplementation([Base class], @selector(test));
//    IMP subIMP = class_getMethodImplementation([Sub class], @selector(test));
//    
//    if (baseIMP == subIMP)
//    {
//        NSLog(@"same");
//    }
//    else
//    {
//        NSLog(@"difference");
//    }
    
    [self.context registerModuleWithClass:[LSCObjectClass class]];
    [self.context evalScriptFromString:@"local obj=Object:create();print(obj);obj=nil;collectgarbage();"];
    
//    [self.context evalScriptFromString:@"print(Object.toString());"];
    
//    [self.context registerModuleWithClass:[TestModule class]];
//    
//    [self.context evalScriptFromString:@"local value = TestModule.call('vim', 100); print(value);"];
}

@end
