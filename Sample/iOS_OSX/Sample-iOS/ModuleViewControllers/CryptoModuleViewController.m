//
//  CryptoModuleViewController.m
//  Sample-iOS
//
//  Created by 冯鸿杰 on 2019/6/16.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "CryptoModuleViewController.h"
#import "LuaScriptCore.h"

@interface CryptoModuleViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation CryptoModuleViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
        
        NSLog(@"lsc exception = %@", message);
        
    }];
    
    [self.context evalScriptFromFile:@"Crypto-Sample.lua"];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch (indexPath.row)
    {
        case 0:
            //MD5
            [self.context evalScriptFromString:@"Crypto_Sample_md5()"];
            break;
        case 1:
            //SHA1
            [self.context evalScriptFromString:@"Crypto_Sample_sha1()"];
            break;
        case 2:
            //HMAC-MD5
            [self.context evalScriptFromString:@"Crypto_Sample_hmacMD5()"];
            break;
        case 3:
            //HMAC-SHA1
            [self.context evalScriptFromString:@"Crypto_Sample_hmacSHA1()"];
            break;
        default:
            break;
    }
}

@end
