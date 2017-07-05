//
//  LSCTuple.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/17.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCTuple.h"
#import "LSCValue_Private.h"
#import "LSCTuple_Private.h"

@implementation LSCTuple

- (instancetype)init
{
    if (self = [super init])
    {
        self.returnValues = [NSMutableArray array];
    }
    return self;
}

- (NSInteger)count
{
    return self.returnValues.count;
}

- (void)addReturnValue:(id)returnValue
{
    [self.returnValues addObject:returnValue];
}

- (id)returnValueForIndex:(NSInteger)index
{
    return self.returnValues[index];
}

- (NSString *)description
{
    return [self.returnValues description];
}

@end
