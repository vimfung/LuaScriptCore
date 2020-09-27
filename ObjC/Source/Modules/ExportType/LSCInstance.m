//
//  LSCInstance.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/8.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCInstance.h"
#import "LSCException.h"
#import "LSCTypeDescription.h"
#import "LSCInstance+Private.h"

@implementation LSCInstance

- (id<LSCValueType>)getPropertyForKey:(NSString *)key
                              context:(nonnull LSCContext *)context
{
    return [self.typeDescription getPropertyWithInstance:self
                                                    name:key
                                                 context:context];
}

- (void)setProperty:(id<LSCValueType>)value
             forKey:(NSString *)key
            context:(nonnull LSCContext *)context
{
    [self.typeDescription setPropertyWithInstance:self
                                             name:key
                                            value:value
                                          context:context];
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"[%@ object<%p>]", self.typeDescription.typeName, self];
}

#pragma mark - Private

- (instancetype)_initWithInstanceId:(NSString *)instanceId
                    typeDescription:(LSCTypeDescription *)typeDescription
                             object:(nullable id)object
                            context:(nonnull LSCContext *)context
{
    if (typeDescription.nativeType && ![object isKindOfClass:typeDescription.nativeType])
    {
        return nil;
    }
    
    if (self = [super init])
    {
        _instanceId = instanceId;
        _typeDescription = typeDescription;
        _object = object;
        _ownerContext = context;
    }
    
    return self;
}

@end
