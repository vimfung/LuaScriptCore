////
//  ViewController.m
//  Sample
//
//  Created by vimfung on 16/7/15.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LuaScriptCore.h"
#import "ViewController.h"
#import "LogModule.h"
#import "LSCTPerson.h"

@interface ViewController ()

/**
 lua上下文
 */
@property(nonatomic, strong) LSCContext *context;

/**
 是否注册方法
 */
@property(nonatomic) BOOL hasRegMethod;

/**
 *  是否注册模块
 */
@property (nonatomic) BOOL hasRegModule;

/**
 *  是否注册类
 */
@property (nonatomic) BOOL hasRegClass;

@end

@implementation ViewController

- (void)viewDidLoad {
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


/**
 解析脚本按钮点击事件

 @param sender 事件对象
 */
- (IBAction)evalScriptButtonClickedHandler:(id)sender
{
  //解析并执行Lua脚本
  LSCValue *retValue =
      [self.context evalScriptFromString:@"print(10);return 'Hello World';"];
  NSLog(@"%@", [retValue toString]);
}


/**
 注册方法按钮点击事件

 @param sender 事件对象
 */
- (IBAction)regMethodClickedHandler:(id)sender
{
  if (!self.hasRegMethod) {
    self.hasRegMethod = YES;

    //注册方法
    [self.context
        registerMethodWithName:@"getDeviceInfo"
                         block:^LSCValue *(NSArray *arguments) {

                           NSMutableDictionary *info =
                               [NSMutableDictionary dictionary];
                           [info setObject:[UIDevice currentDevice].name
                                    forKey:@"deviceName"];
                           [info setObject:[UIDevice currentDevice].model
                                    forKey:@"deviceModel"];
                           [info setObject:[UIDevice currentDevice].systemName
                                    forKey:@"systemName"];
                           [info
                               setObject:[UIDevice currentDevice].systemVersion
                                  forKey:@"systemVersion"];

                           return [LSCValue dictionaryValue:info];

                         }];
  }

  //调用脚本
  [self.context
      evalScriptFromFile:[[NSBundle mainBundle] pathForResource:@"main"
                                                         ofType:@"lua"]];
}


/**
 调用lua方法点击事件

 @param sender 事件对象
 */
- (IBAction)callLuaMethodClickedHandler:(id)sender {
  //加载Lua脚本
  [self.context
      evalScriptFromFile:[[NSBundle mainBundle] pathForResource:@"todo"
                                                         ofType:@"lua"]];

  //调用Lua方法
  LSCValue *value = [self.context callMethodWithName:@"add"
                                           arguments:@[
                                             [LSCValue integerValue:1000],
                                             [LSCValue integerValue:24]
                                           ]];
  NSLog(@"result = %@", [value toNumber]);
}


/**
 注册模块按钮点击事件

 @param sender 事件对象
 */
- (IBAction)registerModuleClickedHandler:(id)sender
{
    if (!self.hasRegModule)
    {
        self.hasRegModule = YES;
        [self.context registerModuleWithClass:[LogModule class]];
    }
    
    [self.context evalScriptFromString:@"LogModule.writeLog('Hello Lua Module!');"];
}


/**
 注册类按钮点击事件

 @param sender 事件对象
 */
- (IBAction)registerClassClickedHandler:(id)sender
{
    if (!self.hasRegClass)
    {
        self.hasRegClass = YES;
        [self.context registerModuleWithClass:[LSCTPerson class]];
    }

    [self.context evalScriptFromString:@"local person = LSCTPerson.super.create();print(person);"];
}

@end
