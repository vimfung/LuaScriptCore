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

@interface ViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    [self.context registerModuleWithClass:[TestModule class]];
    
    [self.context evalScriptFromString:@"local value = TestModule.call('vim', 100); print(value);"];
}

@end
