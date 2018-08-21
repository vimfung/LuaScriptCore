//
//  LSCPath.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCPath.h"

@implementation LSCPath

+ (NSString *)appPath
{
    return [NSBundle mainBundle].resourcePath;
}

+ (NSString *)homePath
{
    return NSHomeDirectory();
}

+ (NSString *)docsPath
{
    return NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
}

+ (NSString *)cachesPath
{
    return NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).firstObject;
}

+ (NSString *)tmpPath
{
    return NSTemporaryDirectory();
}

@end
