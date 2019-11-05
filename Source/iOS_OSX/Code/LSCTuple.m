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
    if (!returnValue)
    {
        returnValue = [NSNull null];
    }
    
    [self.returnValues addObject:returnValue];
}

- (id)returnValueForIndex:(NSInteger)index
{
    id retVal = self.returnValues[index];
    return [retVal isKindOfClass:[NSNull class]] ? nil : retVal;
}

- (NSString *)description
{
    return [self.returnValues description];
}

@end
