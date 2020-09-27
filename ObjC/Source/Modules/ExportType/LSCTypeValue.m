//
//  LSCClassValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCTypeValue.h"
#import "LSCException.h"
#import "LSCTypeDescription.h"
#import "LSCApiAdapter+ExportType.h"
#import "LSCValue.h"

@interface LSCTypeValue ()

@property (nonatomic, strong) LSCTypeDescription *rawValue;

@end

@implementation LSCTypeValue

- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
{
    if ([LSCTypeValue _checkType:typeDescription])
    {
        return [self _initWithClassDescription:typeDescription];
    }
    
    return nil;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCTypeValue _checkType:rawValue])
    {
        return [[LSCTypeValue alloc] _initWithClassDescription:rawValue];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeUserdata ||
        [apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeLightUserdata)
    {
        LSCTypeDescription *typeDesc = [apiAdapter getObjectWithStackIndex:stackIndex context:context];
        return [self createValue:typeDesc];
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushType:self.rawValue context:context];
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

#pragma mark - Private

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[LSCTypeDescription class]];
}

- (instancetype)_initWithClassDescription:(LSCTypeDescription *)classDescription
{
    if (self = [super init])
    {
        self.rawValue = classDescription;
    }
    
    return self;
}

@end
