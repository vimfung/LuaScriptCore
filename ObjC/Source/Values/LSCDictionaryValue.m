//
//  LSCDictionaryValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCDictionaryValue.h"
#import "LSCValue.h"
#import "LSCApiAdapter.h"

@interface LSCDictionaryValue ()

@property (nonatomic, strong) NSMutableDictionary *rawValue;

@end

@implementation LSCDictionaryValue

- (instancetype)initWithDictionary:(NSDictionary *)dictionary
{
    if ([LSCDictionaryValue _checkType:dictionary])
    {
        return [self _initWithDictionary:dictionary];
    }
    
    return nil;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCDictionaryValue _checkType:rawValue])
    {
        return [[LSCDictionaryValue alloc] _initWithDictionary:rawValue];
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
    [apiAdapter pushDictionary:self.rawValue context:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCDictionaryValue class]];
}

- (instancetype)init
{
    return [self _initWithDictionary:@{}];
}

#pragma mark - Private

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[NSDictionary class]];
}

- (instancetype)_initWithDictionary:(NSDictionary *)dictionary
{
    if (self = [super init])
    {
        self.rawValue = [dictionary mutableCopy];
    }
    
    return self;
}

@end
