//
//  PathModuleViewController.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "PathModuleViewController.h"
#import "LuaScriptCore.h"

@interface PathModuleViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation PathModuleViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
        
        NSLog(@"lsc exception = %@", message);
        
    }];
    
    [self.context evalScriptFromFile:@"Path-Sample.lua"];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch (indexPath.row)
    {
        case 0:
            //App Path
            [self.context evalScriptFromString:@"Path_Sample_appPath()"];
            break;
        case 1:
            //Home Path
            [self.context evalScriptFromString:@"Path_Sample_homePath()"];
            break;
        case 2:
            //Docs Path
            [self.context evalScriptFromString:@"Path_Sample_docsPath()"];
            break;
        case 3:
            //Caches Path
            [self.context evalScriptFromString:@"Path_Sample_cachesPath()"];
            break;
        case 4:
            //Tmp Path
            [self.context evalScriptFromString:@"Path_Sample_tmpPath()"];
            break;
        default:
            break;
    }
}

@end
