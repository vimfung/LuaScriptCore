//
//  ModulesViewController.m
//  Sample-OSX
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "ModulesViewController.h"
#import "LuaScriptCore.h"

@interface ModulesViewController () <NSOutlineViewDataSource, NSOutlineViewDelegate>

@property (nonatomic, strong) LSCContext *context;

/**
 模块列表视图
 */
@property (weak) IBOutlet NSOutlineView *outlineView;

@end

@implementation ModulesViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do view setup here.
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
        
        NSLog(@"lsc exception = %@", message);
        
    }];
    
    [self.context evalScriptFromFile:@"Encoding-Sample.lua"];
    [self.context evalScriptFromFile:@"Path-Sample.lua"];
    [self.context evalScriptFromFile:@"HTTP-Sample.lua"];
    [self.context evalScriptFromFile:@"Thread-Sample.lua"];
    [self.context evalScriptFromFile:@"Crypto-Sample.lua"];
}

#pragma mark - NSOutlineViewDataSource

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (!item)
    {
        return 5;
    }
    else if ([item isEqualToString:@"Encoding"])
    {
        return 8;
    }
    else if ([item isEqualToString:@"Path"])
    {
        return 5;
    }
    else if ([item isEqualToString:@"HTTP"])
    {
        return 4;
    }
    else if ([item isEqualToString:@"Thread"])
    {
        return 1;
    }
    else if ([item isEqualToString:@"Crypto"])
    {
        return 4;
    }
    
    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if (!item)
    {
        switch (index)
        {
            case 0:
                return @"Encoding";
            case 1:
                return @"Path";
            case 2:
                return @"HTTP";
            case 3:
                return @"Thread";
            case 4:
                return @"Crypto";
            default:
                return @"";
        }
    }
    else if ([item isEqualToString:@"Encoding"])
    {
        switch (index)
        {
            case 0:
                return @"Url Encode";
            case 1:
                return @"Url Decode";
            case 2:
                return @"Base64 Encode";
            case 3:
                return @"Base64 Decode";
            case 4:
                return @"JSON Encode";
            case 5:
                return @"JSON Decode";
            case 6:
                return @"Hex Encode";
            case 7:
                return @"Hex Decode";
            default:
                return @"";
        }
    }
    else if ([item isEqualToString:@"Path"])
    {
        switch (index)
        {
            case 0:
                return @"App Path";
            case 1:
                return @"Home Path";
            case 2:
                return @"Documents Path";
            case 3:
                return @"Caches Path";
            case 4:
                return @"Tmp Path";
            default:
                return @"";
        }
    }
    else if ([item isEqualToString:@"HTTP"])
    {
        switch (index)
        {
            case 0:
                return @"GET Request";
            case 1:
                return @"POST Request";
            case 2:
                return @"Upload File";
            case 3:
                return @"Download File";
            default:
                return @"";
        }
    }
    else if ([item isEqualToString:@"Thread"])
    {
        switch (index)
        {
            case 0:
                return @"Run Thread";
            default:
                break;
        }
    }
    else if ([item isEqualToString:@"Crypto"])
    {
        switch (index)
        {
            case 0:
                return @"MD5";
            case 1:
                return @"SHA1";
            case 2:
                return @"HMAC-MD5";
            case 3:
                return @"HMAC-SHA1";
            default:
                break;
        }
    }
    
    return @"";
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    if (!item
        || [item isEqualToString:@"Encoding"]
        || [item isEqualToString:@"Path"]
        || [item isEqualToString:@"HTTP"]
        || [item isEqualToString:@"Thread"]
        || [item isEqualToString:@"Crypto"])
    {
        return YES;
    }
    return NO;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    return item;
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    id item = [self.outlineView itemAtRow:self.outlineView.selectedRow];
    if ([item isEqualToString:@"Url Encode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_urlEncode()"];
    }
    else if ([item isEqualToString:@"Url Decode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_urlDecode()"];
    }
    else if([item isEqualToString:@"Base64 Encode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_base64Encode()"];
    }
    else if([item isEqualToString:@"Base64 Decode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_base64Decode()"];
    }
    else if([item isEqualToString:@"JSON Encode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_jsonEndode()"];
    }
    else if([item isEqualToString:@"JSON Decode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_jsonDecode()"];
    }
    else if([item isEqualToString:@"Hex Encode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_hexEncode()"];
    }
    else if([item isEqualToString:@"Hex Decode"])
    {
        [self.context evalScriptFromString:@"Encoding_Sample_hexDecode()"];
    }
    else if ([item isEqualToString:@"App Path"])
    {
        [self.context evalScriptFromString:@"Path_Sample_appPath()"];
    }
    else if ([item isEqualToString:@"Home Path"])
    {
        [self.context evalScriptFromString:@"Path_Sample_homePath()"];
    }
    else if ([item isEqualToString:@"Documents Path"])
    {
        [self.context evalScriptFromString:@"Path_Sample_docsPath()"];
    }
    else if ([item isEqualToString:@"Caches Path"])
    {
        [self.context evalScriptFromString:@"Path_Sample_cachesPath()"];
    }
    else if ([item isEqualToString:@"Tmp Path"])
    {
        [self.context evalScriptFromString:@"Path_Sample_tmpPath()"];
    }
    else if ([item isEqualToString:@"GET Request"])
    {
        [self.context evalScriptFromString:@"HTTP_Sample_get()"];
    }
    else if ([item isEqualToString:@"POST Request"])
    {
        [self.context evalScriptFromString:@"HTTP_Sample_post()"];
    }
    else if ([item isEqualToString:@"Upload File"])
    {
        [self.context evalScriptFromString:@"HTTP_Sample_upload()"];
    }
    else if ([item isEqualToString:@"Download File"])
    {
        [self.context evalScriptFromString:@"HTTP_Sample_download()"];
    }
    else if ([item isEqualToString:@"Run Thread"])
    {
        [self.context evalScriptFromString:@"Thread_Sample_run()"];
    }
    else if ([item isEqualToString:@"MD5"])
    {
        [self.context evalScriptFromString:@"Crypto_Sample_md5()"];
    }
    else if ([item isEqualToString:@"SHA1"])
    {
        [self.context evalScriptFromString:@"Crypto_Sample_sha1()"];
    }
    else if ([item isEqualToString:@"HMAC-MD5"])
    {
        [self.context evalScriptFromString:@"Crypto_Sample_hmacMD5()"];
    }
    else if ([item isEqualToString:@"HMAC-SHA1"])
    {
        [self.context evalScriptFromString:@"Crypto_Sample_hmacSHA1()"];
    }
}

@end
