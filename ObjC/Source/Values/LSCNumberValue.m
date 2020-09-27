//
//  LSCNumberValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCNumberValue.h"
#import "LSCValue.h"
#import "NSNumber+LSC.h"
#import "LSCException.h"
#import "LSCApiAdapter.h"

@interface LSCNumberValue ()

@property (nonatomic, strong) NSNumber *rawValue;

@end

@implementation LSCNumberValue

- (instancetype)initWithNumber:(NSNumber *)numberValue
{
    if ([LSCNumberValue _checkType:numberValue])
    {
        return [self _initWithNumber:numberValue];
    }
    
    return nil;
    
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCNumberValue _checkType:rawValue])
    {
        return [[LSCNumberValue alloc] _initWithNumber:rawValue];
    }
    return nil;
    
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeNumber)
    {
        NSNumber *value = [apiAdapter getNumberWithStackIndex:stackIndex context:context];
        return [self createValue:value];
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushNumber:self.rawValue context:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCNumberValue class]];
}

- (instancetype)init
{
    return [self _initWithNumber:@0];
}

#pragma mark - Private

- (instancetype)_initWithNumber:(NSNumber *)numberValue
{
    if (self = [super init])
    {
        self.rawValue = numberValue;
    }
    
    return self;
}

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[NSNumber class]] && ![(NSNumber *)value isBoolNumber];
}

@end
