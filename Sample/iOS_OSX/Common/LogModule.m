//
//  LogModule.m
//  Sample
//
//  Created by 冯鸿杰 on 16/9/22.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LogModule.h"

@implementation LogModule

- (void)writeLog:(NSString *)message
{
    NSLog(@"** message = %@", message);
}

@end
