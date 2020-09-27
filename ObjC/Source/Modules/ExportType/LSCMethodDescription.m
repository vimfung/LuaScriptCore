//
//  LSCMethodDescription.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCMethodDescription.h"
#import "LSCException.h"
#import "LSCExportTypeRule.h"
#import "LSCTypeDescription+Private.h"
#import "LSCValue.h"
#import "LSCInstance.h"
#import "LSCContext.h"

@implementation LSCMethodDescription


- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
                               selector:(SEL)selector
                        methodSignature:(NSMethodSignature *)methodSignature
                        paramsSignature:(NSString *)paramsSignature
                        resultSignature:(nonnull NSString *)resultSignature
{
    if (self = [super init])
    {
        LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
        
        _typeDescription = typeDescription;
        _selector = selector;
        _methodSignature = methodSignature;
        _paramsSignature = paramsSignature;
        _resultSignature = resultSignature;
        _name = [rule methodNameBySelector:selector];
    }
    return self;
}

- (id<LSCValueType>)invokeWithTarget:(id)target
                           arguments:(NSArray<id<LSCValueType>> *)arguments
                             context:(LSCContext *)context
{
    id<LSCValueType> resultValue = nil;
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:self.methodSignature];
    if (invocation)
    {
        Method m = NULL;
        
        invocation.selector = self.selector;
        
        if ([target isKindOfClass:[LSCTypeDescription class]])
        {
            invocation.target = ((LSCTypeDescription *)target).nativeType;
            m = class_getClassMethod(self.typeDescription.nativeType, invocation.selector);
        }
        else if ([target isKindOfClass:[LSCInstance class]])
        {
            invocation.target = ((LSCInstance *)target).object;
            m = class_getInstanceMethod(self.typeDescription.nativeType, invocation.selector);
        }
        else
        {
            invocation.target = target;
            m = class_getInstanceMethod(self.typeDescription.nativeType, invocation.selector);
        }

        resultValue = [self.typeDescription _callMethod:m
                                             invocation:invocation
                                              arguments:arguments
                                                context:context];
    }
    
    return resultValue;
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

@end
