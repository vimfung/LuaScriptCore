//
//  LSCStringValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCStringValue.h"
#import "LSCValue.h"
#import "LSCException.h"
#import "LSCApiAdapter.h"

@interface LSCStringValue ()

@property (nonatomic, strong) NSData *rawData;

@property (nonatomic, copy) NSString *rawValue;

@end

@implementation LSCStringValue

- (instancetype)initWithString:(NSString *)stringValue
{
    if ([stringValue isKindOfClass:[NSString class]])
    {
        return [self _initWithString:stringValue];
    }
    
    return nil;
}

- (instancetype)initWithData:(NSData *)dataValue
{
    if ([dataValue isKindOfClass:[NSData class]])
    {
        return [self _initWithData:dataValue];
    }
    
    return nil;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCStringValue _checkType:rawValue])
    {
        if ([rawValue isKindOfClass:[NSString class]])
        {
            return [[LSCStringValue alloc] _initWithString:rawValue];
        }
        
        return [[LSCStringValue alloc] _initWithData:rawValue];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeString)
    {
        NSData *data = [apiAdapter getDataWithStackIndex:stackIndex context:context];
        return [self createValue:data];
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushData:self.rawData context:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCStringValue class]];
}

- (instancetype)init
{
    return [self _initWithData:[NSData data]];
}

#pragma mark - Private

- (instancetype)_initWithData:(NSData *)dataValue
{
    if (self = [super init])
    {
        self.rawData = dataValue;
        self.rawValue = [[NSString alloc] initWithData:dataValue encoding:NSUTF8StringEncoding];
    }
    return self;
}

- (instancetype)_initWithString:(NSString *)stringValue
{
    if (self = [super init])
    {
        self.rawValue = stringValue;
        self.rawData = [stringValue dataUsingEncoding:NSUTF8StringEncoding];
    }
    return self;
}

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[NSString class]]
    || [value isKindOfClass:[NSData class]];
}

@end
