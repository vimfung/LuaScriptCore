//
//  Env.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/9.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "Env.h"

@implementation Env

+ (LSCContext *)defaultContext
{
    static LSCContext *context = nil;
    static dispatch_once_t predicate;
    
    dispatch_once(&predicate, ^{
       
        context = [[LSCContext alloc] init];
        
    });
    
    return context;
}

@end
