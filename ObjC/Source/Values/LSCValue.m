//
//  LuaValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCValue.h"
#import "LSCException.h"
#import "LSCApiAdapter.h"

static NSMutableArray<Class<LSCValueType>> *_regValueTypes;

@interface LSCValue ()

/**
 原始值
 */
@property (nonatomic, strong) id rawValue;

@end

@implementation LSCValue

+ (void)registerValueType:(Class<LSCValueType>)valueType
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _regValueTypes = [NSMutableArray array];
    });
    
    if (![_regValueTypes containsObject:valueType])
    {
        [_regValueTypes addObject:valueType];
    }
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([rawValue conformsToProtocol:@protocol(LSCValueType)])
    {
        //如果为值类型则直接返回，无需再创建
        return rawValue;
    }
    
    //进行注册类型的检测，如果存在符合类型则创建对象
    __block id instance = nil;
    [_regValueTypes enumerateObjectsUsingBlock:^(Class<LSCValueType>  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        instance = [obj createValue:rawValue];
        if (instance)
        {
            *stop = YES;
        }
        
    }];
    
    if (!instance)
    {
        LSCValue *value = [[LSCValue alloc] _initWithUnknowValue:rawValue];
        instance = value;
    }
    
    return instance;
}

+ (instancetype)createValueWithContext:(LSCContext *)context
                            stackIndex:(int)stackIndex
{
    //进行注册类型的检测，如果存在符合类型则创建对象
    __block id instance = nil;
    [_regValueTypes enumerateObjectsUsingBlock:^(Class<LSCValueType>  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        instance = [obj createValueWithContext:context stackIndex:stackIndex];
        if (instance)
        {
            *stop = YES;
        }
        
    }];
    
    if (!instance)
    {
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeLightUserdata
            || [apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeUserdata)
        {
            id unknownValue = [apiAdapter getObjectWithStackIndex:stackIndex context:context];
            if (unknownValue)
            {
                instance = [[LSCValue alloc] _initWithUnknowValue:unknownValue];
            }
        }
    }
    
    return instance;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushUnknowObject:self.rawValue context:context];        
}

#pragma mark - Rewrite

- (instancetype)init
{
    //不允许直接初始化对象
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

#pragma mark - Private

- (instancetype)_initWithUnknowValue:(id)value
{
    if (self = [super init])
    {
        self.rawValue = value;
    }
    return self;
}

@end
