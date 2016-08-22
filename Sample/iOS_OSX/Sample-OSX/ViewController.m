//
//  ViewController.m
//  Sample-OSX
//
//  Created by vimfung on 16/8/22.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LuaScriptCore.h"
#import "ViewController.h"

@interface ViewController ()

@property(nonatomic, strong) LSCContext *context;

@property(nonatomic) BOOL hasRegMethod;

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  // Do any additional setup after loading the view.

  self.context = [[LSCContext alloc] init];

  //捕获异常
  [self.context onException:^(NSString *message) {

    NSLog(@"error = %@", message);

  }];
}

- (IBAction)evalScriptButtonClickedHandler:(id)sender {

  //解析并执行Lua脚本
  LSCValue *retValue =
      [self.context evalScriptFromString:@"print(10);return 'Hello World';"];
  NSLog(@"%@", [retValue toString]);
}

- (IBAction)regMethodClcikedHandler:(id)sender {

  if (!self.hasRegMethod) {
    self.hasRegMethod = YES;

    //注册方法
    [self.context registerMethodWithName:@"getDeviceInfo"
                                   block:^LSCValue *(NSArray *arguments) {

                                     NSMutableDictionary *info =
                                         [NSMutableDictionary dictionary];
                                     [info setObject:@"value1" forKey:@"key1"];
                                     [info setObject:@"value2" forKey:@"key2"];
                                     [info setObject:@"value3" forKey:@"key3"];
                                     [info setObject:@"value4" forKey:@"key4"];

                                     return [LSCValue dictionaryValue:info];

                                   }];
  }

  //调用脚本
  [self.context
      evalScriptFromFile:[[NSBundle mainBundle] pathForResource:@"main"
                                                         ofType:@"lua"]];
}

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

@end
