//
//  ViewController.m
//  Sample
//
//  Created by 冯鸿杰 on 16/10/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "ViewController.h"
#import "LuaScriptCore.h"
#import "LogModule.h"
#import "LSCObjectClass.h"

@interface ViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    self.context = [[LSCContext alloc] init];
    [self.context registerModuleWithClass:[LSCObjectClass class]];
    
    [self.context evalScriptFromString:@"Object.subclass('Test'); print (Test.version());"];
}


@end
