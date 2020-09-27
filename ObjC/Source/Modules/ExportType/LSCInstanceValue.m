//
//  LSCInstanceValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/25.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCInstanceValue.h"
#import "LSCInstance+Private.h"
#import "LSCException.h"
#import "LSCApiAdapter+ExportType.h"

@interface LSCInstanceValue ()

@property (nonatomic, strong) LSCInstance *rawValue;

@end

@implementation LSCInstanceValue

#pragma mark - LSCValueType

+ (instancetype)createValue:(_Nullable id)rawValue
{
    if ([LSCInstanceValue _checkType:rawValue])
    {
        return [[LSCInstanceValue alloc] _initWithInstance:rawValue];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context
                            stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeUserdata ||
        [apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeLightUserdata)
    {
        LSCInstance *instance = [apiAdapter getObjectWithStackIndex:stackIndex context:context];
        return [self createValue:instance];
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushInstance:self.rawValue context:context];
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

- (void)dealloc
{
    if (self.rawValue.instanceId)
    {
        //释放引用
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setLuaObjectWithId:self.rawValue.instanceId
                                option:LSCSetLuaObjectOptionRelease
                               context:self.rawValue.ownerContext];
    }
}

#pragma mark - Private

+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[LSCInstance class]];
}

- (instancetype)_initWithInstance:(LSCInstance *)instance
{
    if (self = [super init])
    {
        //引用对象
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setLuaObjectWithId:instance.instanceId
                                option:LSCSetLuaObjectOptionRetain
                               context:instance.ownerContext];
        
        self.rawValue = instance;
    }
    
    return self;
}

@end
