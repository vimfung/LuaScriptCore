//
//  LSCManagedValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/23.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCManagedValue.h"
#import "LSCContext_Private.h"

@interface LSCManagedValue ()

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

/**
 源值对象
 */
@property (nonatomic, strong) LSCValue *source;

@end

@implementation LSCManagedValue

- (instancetype)initWithValue:(LSCValue *)value
                      context:(LSCContext *)context
{
    if (self = [super init])
    {
        self.context = context;
        self.source = value;
        
        [self.context.dataExchanger retainLuaObject:self.source];
    }
    return self;
}

- (void)dealloc
{
    [self.context.dataExchanger releaseLuaObject:self.source];
}

@end
