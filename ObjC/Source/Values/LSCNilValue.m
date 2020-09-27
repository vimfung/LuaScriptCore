//
//  LSCNilValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCNilValue.h"
#import "LSCValue.h"
#import "LSCApiAdapter.h"

@implementation LSCNilValue

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if (!rawValue || [rawValue isKindOfClass:[NSNull class]])
    {
        return [[LSCNilValue alloc] init];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeNil)
    {
        return [[LSCNilValue alloc] init];
    }
    
    return nil;
}

- (id)rawValue
{
    return [NSNull null];
}

- (void)pushWithContext:(LSCContext *)context
{
    [[LSCApiAdapter defaultApiAdapter] pushNilWithContext:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCNilValue class]];
}

@end
