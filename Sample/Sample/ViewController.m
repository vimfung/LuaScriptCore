//
//  ViewController.m
//  Sample
//
//  Created by vimfung on 16/7/15.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "ViewController.h"
#import "LuaScriptCore/LuaScriptCore.h"

@interface ViewController ()

@property (nonatomic, strong) LSCContext *context;

@property (nonatomic) BOOL hasRegMethod;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    
    //捕获异常
    [self.context onException:^(NSString *message) {
        
        NSLog(@"error = %@", message);
        
    }];
    
}

- (UIStatusBarStyle)preferredStatusBarStyle
{
    return UIStatusBarStyleLightContent;
}

- (IBAction)evalScriptButtonClickedHandler:(id)sender
{
    //解析并执行Lua脚本
    LSCValue *retValue = [self.context evalScriptFromString:@"print(10);return 'Hello World';"];
    NSLog(@"%@", [retValue toString]);
}

- (IBAction)regMethodClickedHandler:(id)sender
{
    if (!self.hasRegMethod)
    {
        self.hasRegMethod = YES;
        
        //注册方法
        [self.context registerMethodWithName:@"getDeviceInfo" block:^LSCValue *(NSArray *arguments) {
            
            NSMutableDictionary *info = [NSMutableDictionary dictionary];
            [info setObject:[UIDevice currentDevice].name forKey:@"deviceName"];
            [info setObject:[UIDevice currentDevice].model forKey:@"deviceModel"];
            [info setObject:[UIDevice currentDevice].systemName forKey:@"systemName"];
            [info setObject:[UIDevice currentDevice].systemVersion forKey:@"systemVersion"];
            
            return [LSCValue dictionaryValue:info];
            
        }];
    }
    
    //调用脚本
    [self.context evalScriptFromFile:[[NSBundle mainBundle] pathForResource:@"main" ofType:@"lua"]];
}

- (IBAction)callLuaMethodClickedHandler:(id)sender
{
    //加载Lua脚本
    [self.context evalScriptFromFile:[[NSBundle mainBundle] pathForResource:@"todo" ofType:@"lua"]];
    
    //调用Lua方法
    LSCValue *value = [self.context callMethodWithName:@"add" arguments:@[[LSCValue integerValue:1000], [LSCValue integerValue:24]]];
    NSLog(@"result = %@", [value toNumber]);
}


@end
