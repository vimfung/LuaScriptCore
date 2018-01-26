//
//  LSCExportMethodDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/8.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportMethodDescriptor.h"

@implementation LSCExportMethodDescriptor

- (instancetype)initWithSelector:(SEL)selector
                 methodSignature:(NSMethodSignature *)methodSignature
                 paramsSignature:(NSString *)paramsSignature
{
    if (self = [super init])
    {
        _selector = selector;
        _methodSignature = methodSignature;
        _paramsSignature = paramsSignature;
    }
    return self;
}

- (NSInvocation *)createInvocation
{
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:self.methodSignature];
    invocation.selector = self.selector;
    
    return invocation;
}

@end
