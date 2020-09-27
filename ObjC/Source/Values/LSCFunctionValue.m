//
//  LSCFunctionValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCFunctionValue.h"
#import "LSCException.h"
#import "LSCApiAdapter.h"
#import "LSCFunctionValue+Private.h"
#import "LSCValue.h"

@implementation LSCFunctionValue

- (instancetype)initWithHandler:(LSCFunctionHandler)handler
{
    if (self = [super init])
    {
        self.handler = handler;
    }
    return self;
}

- (id<LSCValueType>)invokeWithArguments:(nullable NSArray<id<LSCValueType>> *)arguments
                                context:(nonnull LSCContext *)context
{
    if (self.handler)
    {
        return self.handler(arguments);
    }
    else if (self.functionId)
    {
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        return [apiAdapter callFunctionWithId:self.functionId arguments:arguments context:context];
    }
    
    return nil;
}

- (BOOL)nativeFunction
{
    return self.handler ? YES : NO;
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeFunction)
    {
        NSString *objectId = [apiAdapter getLuaObjectIdWithStackIndex:stackIndex context:context];
        if (objectId)
        {
            return [[LSCFunctionValue alloc] _initWithFunctionId:objectId
                                                         context:context];
        }
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    
    if (self.handler)
    {
        [apiAdapter pushFunction:self context:context];
    }
    else if(self.functionId)
    {
        [apiAdapter pushLuaObjectWithId:self.functionId context:context];
    }
    else
    {
        [apiAdapter pushNilWithContext:context];
    }
}

- (id)rawValue
{
    return self;
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCFunctionValue class]];
}

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

- (void)dealloc
{
    if (self.functionId)
    {
        //释放引用
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setLuaObjectWithId:self.functionId
                                option:LSCSetLuaObjectOptionRelease
                               context:self.ownerContext];
    }
}

#pragma mark - Private

- (instancetype)_initWithFunctionId:(NSString *)functionId
                            context:(LSCContext *)context
{
    if (self = [super init])
    {
        //引用对象
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setLuaObjectWithId:functionId option:LSCSetLuaObjectOptionRetain context:context];
        
        self.functionId = functionId;
        self.ownerContext = context;
    }
    
    return self;
}

@end
