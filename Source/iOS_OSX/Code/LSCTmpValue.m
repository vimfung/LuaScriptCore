//
//  LSCTmpValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/12/13.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCTmpValue.h"
#import "LSCEngineAdapter.h"

#import "LSCValue_Private.h"
#import "LSCSession_Private.h"
#import "LSCContext_Private.h"

@interface LSCTmpValue ()

/**
 栈索引
 */
@property (nonatomic) NSInteger index;

/**
 解析后的Value
 */
@property (nonatomic, strong) LSCValue *parsedValue;

@end

@implementation LSCTmpValue

- (instancetype)initWithContext:(LSCContext *)context
                          index:(NSInteger)index
{
    if (self = [super init])
    {
        lua_State *state = context.currentSession.state;
        self.context = context;
        
        [self.context.optQueue performAction:^{
            self.index = [LSCEngineAdapter absIndex:(int)index state:state];
        }];
    }
    
    return self;
}

- (LSCValueType)valueType
{
    [self _parseValue];
    return self.parsedValue.valueType;
}

- (id)toObject
{
    [self _parseValue];
    return [self.parsedValue toObject];
}

- (NSString *)toString
{
    [self _parseValue];
    return [self.parsedValue toString];
}

- (NSNumber *)toNumber
{
    [self _parseValue];
    return [self.parsedValue toNumber];
}

- (NSInteger)toInteger
{
    [self _parseValue];
    return [self.parsedValue toInteger];
}

- (double)toDouble
{
    [self _parseValue];
    return [self.parsedValue toDouble];
}

- (BOOL)toBoolean
{
    [self _parseValue];
    return [self.parsedValue toBoolean];
}

- (NSData *)toData
{
    [self _parseValue];
    return [self.parsedValue toData];
}

- (NSArray *)toArray
{
    [self _parseValue];
    return [self.parsedValue toArray];
}

- (NSDictionary *)toDictionary
{
    [self _parseValue];
    return [self.parsedValue toDictionary];
}

- (LSCPointer *)toPointer
{
    [self _parseValue];
    return [self.parsedValue toPointer];
}

- (LSCFunction *)toFunction
{
    [self _parseValue];
    return [self.parsedValue toFunction];
}

- (LSCTuple *)toTuple
{
    [self _parseValue];
    return [self.parsedValue toTuple];
}

- (LSCExportTypeDescriptor *)toType
{
    [self _parseValue];
    return [self.parsedValue toType];
}

- (void)pushWithContext:(LSCContext *)context
{
    if (self.parsedValue)
    {
        [self.parsedValue pushWithContext:context];
    }
    else
    {
        [self.context.optQueue performAction:^{
            lua_State *state = self.context.currentSession.state;
            [LSCEngineAdapter pushValue:(int)self.index state:state];
        }];
    }
}

#pragma mark - Private

/**
 解析值
 */
- (void)_parseValue
{
    if (!self.parsedValue)
    {
        self.parsedValue = [LSCValue valueWithContext:self.context atIndex:self.index];
    }
}

@end
