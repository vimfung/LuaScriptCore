//
//  ThreadModuleViewController.m
//  Sample
//
//  Created by 冯鸿杰 on 2019/1/1.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "ThreadModuleViewController.h"
#import "LuaScriptCore.h"

@interface ThreadModuleViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation ThreadModuleViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
        
        NSLog(@"lsc exception = %@", message);
        
    }];
    
    [self.context evalScriptFromFile:@"Thread-Sample.lua"];
}

#pragma mark - Table view data source

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch (indexPath.row)
    {
        case 0:
            //Run Thread
            [self.context evalScriptFromString:@"Thread_Sample_run()"];
            break;
        default:
            break;
    }
}

@end
