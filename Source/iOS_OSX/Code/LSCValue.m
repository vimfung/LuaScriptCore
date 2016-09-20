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
@property(nonatomic, strong) id valueContainer;

/**
 *  数值类型
 */
@property(nonatomic) LSCValueType valueType;

@end

@implementation LSCValue

+ (instancetype)nilValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeNil value:[NSNull null]];
}

+ (instancetype)numberValue:(NSNumber *)numberValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeNumber value:numberValue];
}

+ (instancetype)booleanValue:(BOOL)boolValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeBoolean value:@(boolValue)];
}

+ (instancetype)stringValue:(NSString *)stringValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeString
                                    value:[stringValue copy]];
}

+ (instancetype)integerValue:(NSInteger)integerValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeInteger value:@(integerValue)];
}

+ (instancetype)arrayValue:(NSArray *)arrayValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeArray value:arrayValue];
}

+ (instancetype)dictionaryValue:(NSDictionary *)dictionaryValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeMap value:dictionaryValue];
}

+ (instancetype)dataValue:(NSData *)dataValue
{
    return [[LSCValue alloc] initWithType:LSCValueTypeData value:dataValue];
}

+ (instancetype)objectValue:(id)objectValue
{
    if ([objectValue isKindOfClass:[NSDictionary class]])
    {
        return [self dictionaryValue:objectValue];
    }
    else if ([objectValue isKindOfClass:[NSArray class]])
    {
        return [self arrayValue:objectValue];
    }
    else if ([objectValue isKindOfClass:[NSNumber class]])
    {
        return [self numberValue:objectValue];
    }
    else if ([objectValue isKindOfClass:[NSString class]])
    {
        return [self stringValue:objectValue];
    }
    else if ([objectValue isKindOfClass:[NSData class]])
    {
        return [self dataValue:objectValue];
    }
    
    return [self nilValue];
}

- (instancetype)init
{
    if (self = [super init])
    {
        self.valueContainer = [NSNull null];
    }
    
    return self;
}

- (void)pushWithState:(lua_State *)state
{
    switch (self.valueType)
    {
        case LSCValueTypeInteger:
            lua_pushinteger(state, [self.valueContainer integerValue]);
            break;
        case LSCValueTypeNumber:
            lua_pushnumber(state, [self.valueContainer doubleValue]);
            break;
        case LSCValueTypeNil:
            lua_pushnil(state);
            break;
        case LSCValueTypeString:
            lua_pushstring(state, [self.valueContainer UTF8String]);
            break;
        case LSCValueTypeBoolean:
            lua_pushboolean(state, [self.valueContainer boolValue]);
            break;
        case LSCValueTypeArray:
        case LSCValueTypeMap:
        {
            [self pushTable:state value:self.valueContainer];
            break;
        }
        case LSCValueTypeData:
        {
            lua_pushlstring(state, [self.valueContainer bytes],
                            [self.valueContainer length]);
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
        case LSCValueTypeNumber:
        case LSCValueTypeInteger:
        case LSCValueTypeBoolean:
            return self.valueContainer;
        case LSCValueTypeString:
            return @([(NSString *)self.valueContainer doubleValue]);
        default:
            return @((NSInteger)self.valueContainer);
    }
}

- (NSInteger)toInteger
{
    switch (self.valueType)
    {
        case LSCValueTypeNumber:
        case LSCValueTypeInteger:
        case LSCValueTypeBoolean:
            return [(NSNumber *)self.valueContainer integerValue];
        case LSCValueTypeString:
            return [(NSString *)self.valueContainer integerValue];
        default:
            return (NSInteger)self.valueContainer;
    }
}

- (double)toDouble
{
    switch (self.valueType)
    {
        case LSCValueTypeNumber:
        case LSCValueTypeInteger:
        case LSCValueTypeBoolean:
            return [(NSNumber *)self.valueContainer doubleValue];
        case LSCValueTypeString:
            return [(NSString *)self.valueContainer doubleValue];
        default:
            return (double)(NSInteger)self.valueContainer;
    }
}

- (BOOL)toBoolean
{
    switch (self.valueType)
    {
        case LSCValueTypeNumber:
        case LSCValueTypeInteger:
        case LSCValueTypeBoolean:
            return [(NSNumber *)self.valueContainer boolValue];
        case LSCValueTypeString:
            return [(NSString *)self.valueContainer boolValue];
        default:
            return (BOOL)self.valueContainer;
    }
}

- (NSData *)toData
{
    if (self.valueType == LSCValueTypeData)
    {
        return self.valueContainer;
    }
    
    return nil;
}

- (NSArray *)toArray
{
    if (self.valueType == LSCValueTypeArray)
    {
        return self.valueContainer;
    }
    
    return nil;
}

- (NSDictionary *)toDictionary
{
    if (self.valueType == LSCValueTypeMap)
    {
        return self.valueContainer;
    }
    
    return nil;
}

- (NSString *)description
{
    return [self.valueContainer description];
}

#pragma mark - Private

+ (LSCValue *)valueWithState:(lua_State *)state atIndex:(NSInteger)index
{
    LSCValue *value = nil;
    
    switch (lua_type(state, (int)index))
    {
        case LUA_TNIL:
        {
            value = [LSCValue nilValue];
            break;
        }
        case LUA_TBOOLEAN:
        {
            value = [LSCValue booleanValue:lua_toboolean(state, (int)index)];
            break;
        }
        case LUA_TNUMBER:
        {
            value = [LSCValue numberValue:@(lua_tonumber(state, (int)index))];
            break;
        }
        case LUA_TSTRING:
        {
            
            size_t len = 0;
            const char *bytes = lua_tolstring(state, (int)index, &len);
            
            NSString *strValue =
            [NSString stringWithCString:bytes encoding:NSUTF8StringEncoding];
            if (strValue)
            {
                //为NSString
                value = [LSCValue stringValue:strValue];
            }
            else
            {
                //为NSData
                NSData *data = [NSData dataWithBytes:bytes length:len];
                value = [LSCValue dataValue:data];
            }
            
            break;
        }
        case LUA_TTABLE:
        {
            NSMutableDictionary *dictValue = [NSMutableDictionary dictionary];
            NSMutableArray *arrayValue = [NSMutableArray array];
            
            lua_pushnil(state);
            while (lua_next(state, -2))
            {
                LSCValue *value = [self valueWithState:state atIndex:-1];
                LSCValue *key = [self valueWithState:state atIndex:-2];
                
                if (arrayValue)
                {
                    if (key.valueType != LSCValueTypeNumber)
                    {
                        //非数组对象，释放数组
                        arrayValue = nil;
                    }
                    else if (key.valueType == LSCValueTypeNumber)
                    {
                        NSInteger index = [[key toNumber] integerValue];
                        if (index <= 0)
                        {
                            //非数组对象，释放数组
                            arrayValue = nil;
                        }
                        else if (index - 1 != arrayValue.count)
                        {
                            //非数组对象，释放数组
                            arrayValue = nil;
                        }
                        else
                        {
                            [arrayValue addObject:[value toObject]];
                        }
                    }
                }
                
                [dictValue setObject:[value toObject] forKey:[key toString]];
                
                lua_pop(state, 1);
            }
            
            if (arrayValue)
            {
                value = [LSCValue arrayValue:arrayValue];
            }
            else
            {
                value = [LSCValue dictionaryValue:dictValue];
            }
            
            break;
        }
        default:
        {
            //默认为nil
            value = [LSCValue nilValue];
            break;
        }
    }
    
    return value;
}

/**
 *  初始化值对象
 *
 *  @param type  类型
 *  @param value 值
 *
 *  @return 值对象
 */
- (instancetype)initWithType:(LSCValueType)type value:(id)value
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
- (void)pushTable:(lua_State *)state value:(id)value
{
    __weak LSCValue *theValue = self;
    
    if ([value isKindOfClass:[NSDictionary class]])
    {
        lua_newtable(state);
        [(NSDictionary *)value
         enumerateKeysAndObjectsUsingBlock:^(id _Nonnull key, id _Nonnull obj, BOOL *_Nonnull stop) {
             
             [theValue pushTable:state value:obj];
             lua_setfield(state, -2, [[NSString stringWithFormat:@"%@", key] UTF8String]);
             
         }];
    }
    else if ([value isKindOfClass:[NSArray class]])
    {
        lua_newtable(state);
        [(NSArray *)value enumerateObjectsUsingBlock:^(id _Nonnull obj, NSUInteger idx, BOOL *_Nonnull stop) {
             
             // lua数组下标从1开始
             [theValue pushTable:state value:obj];
             lua_rawseti(state, -2, idx + 1);
             
         }];
    }
    else if ([value isKindOfClass:[NSNumber class]])
    {
        lua_pushnumber(state, [value doubleValue]);
    }
    else if ([value isKindOfClass:[NSString class]])
    {
        lua_pushstring(state, [value UTF8String]);
    }
}

@end
