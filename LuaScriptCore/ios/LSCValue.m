//
//  LUAValue.m
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "LSCValue_Private.h"
#import "lauxlib.h"

@interface LSCValue ()

/**
 *  数值容器
 */
@property (nonatomic, strong) id valueContainer;

/**
 *  数值类型
 */
@property (nonatomic) LUAValueType valueType;

@end

@implementation LSCValue

+ (instancetype)nilValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeNil value:[NSNull null]];
}

+ (instancetype)numberValue:(NSNumber *)numberValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeNumber value:numberValue];
}

+ (instancetype)booleanValue:(BOOL)boolValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeBoolean value:@(boolValue)];
}

+ (instancetype)stringValue:(NSString *)stringValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeString value:[stringValue copy]];
}

+ (instancetype)integerValue:(NSInteger)integerValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeInteger value:@(integerValue)];
}

+ (instancetype)arrayValue:(NSArray *)arrayValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeTable value:arrayValue];
}

+ (instancetype)dictionaryValue:(NSDictionary *)dictionaryValue
{
    return [[LSCValue alloc] initWithType:LUAValueTypeTable value:dictionaryValue];
}

- (instancetype)init
{
    if (self = [super init])
    {
        self.valueContainer = [NSNull null];
    }
    
    return self;
}

- (void)pushWithState:(NameDef(lua_State) *)state
{
    switch (self.valueType)
    {
        case LUAValueTypeInteger:
            NameDef(lua_pushinteger)(state, [self.valueContainer integerValue]);
            break;
        case LUAValueTypeNumber:
            NameDef(lua_pushnumber)(state, [self.valueContainer doubleValue]);
            break;
        case LUAValueTypeNil:
            NameDef(lua_pushnil)(state);
            break;
        case LUAValueTypeString:
            NameDef(lua_pushstring)(state, [self.valueContainer UTF8String]);
            break;
        case LUAValueTypeBoolean:
            NameDef(lua_pushboolean)(state, [self.valueContainer boolValue]);
            break;
        case LUAValueTypeTable:
        {
            [self pushTable:state value:self.valueContainer];
            break;
        }
        default:
            break;
    }
}

- (id)toObject
{
    return self.valueContainer;
}

- (NSString *)toString
{
    return [NSString stringWithFormat:@"%@", self.valueContainer];
}

- (NSNumber *)toNumber
{
    switch (self.valueType)
    {
        case LUAValueTypeNumber:
        case LUAValueTypeInteger:
        case LUAValueTypeBoolean:
            return self.valueContainer;
        case LUAValueTypeString:
            return @([(NSString *)self.valueContainer doubleValue]);
        default:
            return nil;
    }
}

- (NSString *)description
{
    return [self.valueContainer description];
}

#pragma mark - Private

/**
 *  初始化值对象
 *
 *  @param type  类型
 *  @param value 值
 *
 *  @return 值对象
 */
- (instancetype)initWithType:(LUAValueType)type value:(id)value
{
    if (self = [super init])
    {
        self.valueType = type;
        self.valueContainer = value;
    }
    
    return self;
}

/**
 *  压入一个Table类型
 *
 *  @param state Lua解析器
 *  @param value 值
 */
- (void)pushTable:(NameDef(lua_State) *)state value:(id)value
{
    __weak LSCValue *theValue = self;

    if ([value isKindOfClass:[NSDictionary class]])
    {
        lua_newtable(state);
        [(NSDictionary *)value enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
            
            [theValue pushTable:state value:obj];
            NameDef(lua_setfield)(state, - 2, [[NSString stringWithFormat:@"%@", key] UTF8String]);
            
        }];
    }
    else if ([value isKindOfClass:[NSArray class]])
    {
        lua_newtable(state);
        [(NSArray *)value enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
           
            [theValue pushTable:state value:obj];
            NameDef(lua_setfield)(state, -2, [[NSString stringWithFormat:@"%lu", idx] UTF8String]);
            
        }];
    }
    else if ([value isKindOfClass:[NSNumber class]])
    {
        NameDef(lua_pushnumber)(state, [value doubleValue]);
    }
    else if ([value isKindOfClass:[NSString class]])
    {
        NameDef(lua_pushstring)(state, [value UTF8String]);
    }
}

@end
