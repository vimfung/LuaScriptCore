//
//  LSCContextConfig.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCContextConfig.h"

@implementation LSCContextConfig

+ (instancetype)defaultConfig
{
    static LSCContextConfig *config = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        config = [[LSCContextConfig alloc] init];
    });
    
    return config;
}

@end
