//
//  LSCArrayValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCArrayValue.h"
#import "LSCValue.h"
#import "LSCApiAdapter.h"

@interface LSCArrayValue ()

@property (nonatomic, strong) NSMutableArray *rawValue;

@end

@implementation LSCArrayValue

- (instancetype)initWithArray:(NSArray *)array
{
    if ([LSCArrayValue _checkType:array])
    {
        return [self _initWithArray:array];
    }
    
    return nil;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCArrayValue _checkType:rawValue])
    {
        return [[LSCArrayValue alloc] _initWithArray:rawValue];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeTable)
    {
        id tableValue = [apiAdapter getTableWithStackIndex:stackIndex context:context];
        return [self createValue:tableValue];
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushArray:self.rawValue context:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCArrayValue class]];
}

- (instancetype)init
{
    return [self _initWithArray:@[]];
}

#pragma mark - Private

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[NSArray class]];
}

- (instancetype)_initWithArray:(NSArray *)array
{
    if (self = [super init])
    {
        self.rawValue = [array mutableCopy];
    }
    
    return self;
}

@end
