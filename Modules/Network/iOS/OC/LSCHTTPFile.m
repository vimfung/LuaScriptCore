//
//  LSCHTTPFile.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCHTTPFile.h"

@implementation LSCHTTPFile

- (NSString *)mimeType
{
    if (!_mimeType)
    {
        return @"application/octet-stream";
    }
    
    return _mimeType;
}

@end
