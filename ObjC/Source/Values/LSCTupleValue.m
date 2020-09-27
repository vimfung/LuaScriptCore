//
//  LSCTupleValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCTupleValue.h"
#import "LSCValue.h"
#import "LSCApiAdapter.h"

@interface LSCTupleValue ()

/**
 对象数组
 */
@property (nonatomic, strong) NSMutableArray<id<LSCValueType>> *objectArray;

@end

@implementation LSCTupleValue

- (instancetype)initWithContext:(LSCContext *)context
                     stackIndex:(int)stackIndex
                          count:(NSInteger)count
{
    if (self = [self init])
    {
        for (int i = 1; i <= count; i++)
        {
            id<LSCValueType> value = [LSCValue createValueWithContext:context
                                                           stackIndex:stackIndex + i];
            [self.objectArray addObject:value];
        }
    }

    return self;
}

- (void)addObject:(id)value
{
    id<LSCValueType> obj = [LSCValue createValue:value];
    [self.objectArray addObject:obj];
}

- (void)removeObjectAtIndex:(NSUInteger)index
{
    [self.objectArray removeObjectAtIndex:index];
}

- (id)objectAtIndex:(NSUInteger)index
{
    return [[self.objectArray objectAtIndex:index] rawValue];
}

- (NSInteger)count
{
    return self.objectArray.count;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context
                            stackIndex:(int)stackIndex
{
    //不参与检测和创建对象
    return nil;
}

- (id)rawValue
{
    return self;
}

- (void)pushWithContext:(LSCContext *)context
{
    [self.rawValue enumerateObjectsUsingBlock:^(id<LSCValueType> obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [obj pushWithContext:context];
    }];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCTupleValue class]];
}

- (instancetype)init
{
    if (self = [super init])
    {
        self.objectArray = [NSMutableArray array];
    }
    return self;
}

@end
