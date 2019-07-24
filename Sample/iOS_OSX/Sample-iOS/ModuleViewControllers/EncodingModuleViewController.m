//
//  EncodingModuleViewController.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "EncodingModuleViewController.h"
#import "LuaScriptCore.h"

@interface EncodingModuleViewController ()

@property (nonatomic, strong) LSCContext *context;

@end

@implementation EncodingModuleViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
        
        NSLog(@"lsc exception = %@", message);
        
    }];
    
    [self.context evalScriptFromFile:@"Encoding-Sample.lua"];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch (indexPath.row)
    {
        case 0:
            //Url Encode
            [self.context evalScriptFromString:@"Encoding_Sample_urlEncode()"];
            break;
        case 1:
            //Url Decode
            [self.context evalScriptFromString:@"Encoding_Sample_urlDecode()"];
            break;
        case 2:
            //Base64 Encode
            [self.context evalScriptFromString:@"Encoding_Sample_base64Encode()"];
            break;
        case 3:
            //Base64 Decode
            [self.context evalScriptFromString:@"Encoding_Sample_base64Decode()"];
            break;
        case 4:
            //JSON Encode
            [self.context evalScriptFromString:@"Encoding_Sample_jsonEndode()"];
            break;
        case 5:
            //JSON Decode
            [self.context evalScriptFromString:@"Encoding_Sample_jsonDecode()"];
            break;
        case 6:
            //Hex Encode
            [self.context evalScriptFromString:@"Encoding_Sample_hexEncode()"];
            break;
        case 7:
            //Hex Decode
            [self.context evalScriptFromString:@"Encoding_Sample_hexDecode()"];
            break;
        default:
            break;
    }
}

@end
