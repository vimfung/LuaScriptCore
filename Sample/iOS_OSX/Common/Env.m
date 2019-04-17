//
//  Env.m
//  Sample
//
//  Created by 冯鸿杰 on 2019/3/11.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "Env.h"

@implementation Env

+ (LSCContext *)defaultContext
{
    static LSCContext *context = nil;
    static dispatch_once_t predicate;
    
    dispatch_once(&predicate, ^{
        
        context = [[LSCContext alloc] init];
        [context onException:^(NSString *message) {
           
            NSLog(@"exception = %@", message);
            
        }];
        
    });
    
    return context;
}

+ (LSCScriptController *)runScriptConfig
{
    static LSCScriptController *config = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        config = [[LSCScriptController alloc] init];
        config.timeout = 5;
    });
    
    return config;
}

@end
