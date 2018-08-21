//
//  Encoding.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCEncoding.h"

@implementation LSCEncoding

+ (NSString *)urlEncode:(NSString *)string
{
    if (![string isKindOfClass:[NSString class]])
    {
        return nil;
    }
    
    NSString *newString = CFBridgingRelease(CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault,
                                                                                    (CFStringRef)string,
                                                                                    NULL,
                                                                                    CFSTR("!*'();:@&=+$,/?%#[]"),
                                                                                    CFStringConvertNSStringEncodingToEncoding(NSUTF8StringEncoding)));

    return newString;
}

+ (NSString *)urlDecode:(NSString *)string
{
    if (![string isKindOfClass:[NSString class]])
    {
        return nil;
    }
    
    NSString *newString = [[string stringByReplacingOccurrencesOfString:@"+" withString:@" "] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    
    if (newString)
    {
        return newString;
    }
    
    return nil;
}

@end
