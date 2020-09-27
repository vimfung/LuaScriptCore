//
//  LSCBooleanValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCBooleanValue.h"
#import "LSCValue.h"
#import "NSNumber+LSC.h"
#import "LSCException.h"
#import "LSCApiAdapter.h"

@interface LSCBooleanValue ()

/**
 布尔值
 */
@property (nonatomic, strong) NSNumber *rawValue;

@end

@implementation LSCBooleanValue

- (instancetype)initWithBoolNumber:(NSNumber *)boolNumber
{
    if ([LSCBooleanValue _checkType:boolNumber])
    {
        return [self _initWithBoolNumber:boolNumber];
    }
    
    return nil;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCBooleanValue _checkType:rawValue])
    {
        return [[LSCBooleanValue alloc] _initWithBoolNumber:rawValue];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeBoolean)
    {
        NSNumber *value = [apiAdapter getBooleanWithStackIndex:stackIndex context:context];
        return [self createValue:value];
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushBoolean:self.rawValue context:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCBooleanValue class]];
}

- (instancetype)init
{
    return [self _initWithBoolNumber:@NO];
}

#pragma mark - Private

- (instancetype)_initWithBoolNumber:(NSNumber *)boolNumber
{
    if (self = [super init])
    {
        self.rawValue = boolNumber;
    }
    
    return self;
}

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[NSNumber class]] && [(NSNumber *)value isBoolNumber];
}

@end
