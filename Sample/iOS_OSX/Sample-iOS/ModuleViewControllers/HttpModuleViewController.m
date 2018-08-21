//
//  HttpModuleViewController.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "HttpModuleViewController.h"
#import "LuaScriptCore.h"

@interface HttpModuleViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation HttpModuleViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
        
        NSLog(@"lsc exception = %@", message);
        
    }];
    
    [self.context evalScriptFromFile:@"HTTP-Sample.lua"];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch (indexPath.row)
    {
        case 0:
            //Get
            [self.context evalScriptFromString:@"HTTP_Sample_get()"];
            break;
        case 1:
            //Post
            [self.context evalScriptFromString:@"HTTP_Sample_post()"];
            break;
        case 2:
            //Upload
            [self.context evalScriptFromString:@"HTTP_Sample_upload()"];
            break;
        case 3:
            //Download
            [self.context evalScriptFromString:@"HTTP_Sample_download()"];
            break;
        default:
            break;
    }
}

@end
