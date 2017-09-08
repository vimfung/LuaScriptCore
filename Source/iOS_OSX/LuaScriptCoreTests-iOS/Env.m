//
//  Env.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/9.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "Env.h"
#import "LSCContextConfig.h"

@implementation Env

+ (LSCContext *)defaultContext
{
    static LSCContext *context = nil;
    static dispatch_once_t predicate;
    
    dispatch_once(&predicate, ^{
       
        LSCContextConfig *config = [[LSCContextConfig alloc] init];
        config.manualImportClassEnabled = YES;
        context = [[LSCContext alloc] initWithConfig:config];
        
    });
    
    return context;
}

@end
