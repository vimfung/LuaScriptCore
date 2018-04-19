//
//  LUAValue.m
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "LSCValue_Private.h"
#import "LSCContext_Private.h"
#import "LSCFunction_Private.h"
#import "LSCDataExchanger_Private.h"
#import "LSCPointer.h"
#import "LSCTuple_Private.h"
#import "LSCManagedObjectProtocol.h"
#import "LSCExportTypeDescriptor.h"
#import "LSCTmpValue.h"

@implementation LSCValue

+ (instancetype)nilValue
{
    return [[self alloc] initWithType:LSCValueTypeNil value:nil];
}

+ (instancetype)numberValue:(NSNumber *)numberValue
{
    return [[self alloc] initWithType:LSCValueTypeNumber value:numberValue];
}

+ (instancetype)booleanValue:(BOOL)boolValue
{
    return [[self alloc] initWithType:LSCValueTypeBoolean value:@(boolValue)];
}

+ (instancetype)stringValue:(NSString *)stringValue
{
    return [[self alloc] initWithType:LSCValueTypeString
                                value:[stringValue copy]];
}

+ (instancetype)integerValue:(NSInteger)integerValue
{
    return [[self alloc] initWithType:LSCValueTypeInteger value:@(integerValue)];
}

+ (instancetype)arrayValue:(NSArray *)arrayValue
{
    return [[self alloc] initWithType:LSCValueTypeArray value:arrayValue];
}

+ (instancetype)dictionaryValue:(NSDictionary *)dictionaryValue
{
    return [[self alloc] initWithType:LSCValueTypeMap value:dictionaryValue];
}

+ (instancetype)dataValue:(NSData *)dataValue
{
    return [[self alloc] initWithType:LSCValueTypeData value:dataValue];
}

+ (instancetype)objectValue:(id)objectValue
{
    if (objectValue)
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
        else if ([objectValue isKindOfClass:[LSCFunction class]])
        {
            return [self functionValue:objectValue];
        }
        else if ([objectValue isKindOfClass:[LSCValue class]])
        {
            //不做任何封装，直接返回
            return objectValue;
        }
        else if ([objectValue isKindOfClass:[LSCTuple class]])
        {
            return [self tupleValue:objectValue];
        }
        else if ([objectValue isKindOfClass:[LSCExportTypeDescriptor class]])
        {
            return [self typeValue:objectValue];
        }
        else
        {
            return [[self alloc] initWithType:LSCValueTypeObject value:objectValue];
        }
    }
    
    return [self nilValue];
}

+ (instancetype)pointerValue:(LSCPointer *)pointerValue
{
    return [[self alloc] initWithType:LSCValueTypePtr value:pointerValue];
}

+ (instancetype)functionValue:(LSCFunction *)functionValue
{
    return [[self alloc] initWithType:LSCValueTypeFunction value:functionValue];
}

+ (instancetype)tupleValue:(LSCTuple *)tupleValue
{
    return [[self alloc] initWithType:LSCValueTypeTuple value:tupleValue];
}

+ (instancetype)typeValue:(LSCExportTypeDescriptor *)typeDescriptor
{
    return [[self alloc] initWithType:LSCValueTypeClass value:typeDescriptor];
}

- (void)dealloc
{
    if (self.hasManagedObject)
    {
        self.hasManagedObject = false;
        [self.context.dataExchanger releaseLuaObject:self];
    }
}

- (void)pushWithContext:(LSCContext *)context
{
    [context.dataExchanger pushStackWithValue:self];
}

- (id)toObject
{
    if (self.valueType == LSCValueTypePtr)
    {
        return (__bridge id)([(LSCPointer *)self.valueContainer value] -> value);
    }
    
    return self.valueContainer;
}

- (NSString *)toString
{
    if (self.valueType == LSCValueTypePtr)
    {
        return nil;
    }
    
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
        case LSCValueTypePtr:
            return nil;
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
        case LSCValueTypePtr:
            return (int)[[self toPointer] value];
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
        case LSCValueTypePtr:
            return 0.0;
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
        case LSCValueTypePtr:
            return NO;
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
    else if (self.valueType == LSCValueTypeString)
    {
        return [(NSString *)self.valueContainer dataUsingEncoding:NSUTF8StringEncoding];
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

- (LSCPointer *)toPointer
{
    if (self.valueType == LSCValueTypePtr)
    {
        return self.valueContainer;
    }
    
    return [[LSCPointer alloc] initWithPtr:(__bridge const void *)(self.valueContainer)];
}

- (LSCFunction *)toFunction
{
    if (self.valueType == LSCValueTypeFunction)
    {
        return self.valueContainer;
    }
    
    return nil;
}

- (LSCTuple *)toTuple
{
    if (self.valueType == LSCValueTypeTuple)
    {
        return self.valueContainer;
    }
    
    return nil;
}

- (LSCExportTypeDescriptor *)toType
{
    if (self.valueType == LSCValueTypeClass)
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

+ (LSCValue *)valueWithContext:(LSCContext *)context atIndex:(NSInteger)index
{
    LSCValue *value = [context.dataExchanger valueByStackIndex:(int)index];
    //将对象内存管理权交给LSCValue
    [value managedObjectWithContext:context];
    return value;
}

+ (LSCValue *)tmpValueWithContext:(LSCContext *)context atIndex:(NSInteger)index
{
    return [[LSCTmpValue alloc] initWithContext:context index:index];
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

- (void)managedObjectWithContext:(LSCContext *)context
{
    self.context = context;
    if (!self.hasManagedObject)
    {
        self.hasManagedObject = YES;
        [self.context.dataExchanger retainLuaObject:self];
    }
}

@end
