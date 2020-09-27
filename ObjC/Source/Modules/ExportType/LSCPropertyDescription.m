//
//  LSCPropertyDescription.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/22.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCPropertyDescription.h"
#import "LSCException.h"
#import "LSCInstance.h"
#import "LSCContext.h"
#import "LSCTypeDescription+Private.h"
#import "LSCFunctionValue.h"
#import "LSCInstanceValue.h"
#import "LSCValue.h"

@interface LSCPropertyDescription ()

/**
 获取方法
 */
@property (nonatomic) SEL getter;
@property (nonatomic) LSCFunctionValue *getterFunc;

/**
 设置方法
 */
@property (nonatomic) SEL setter;
@property (nonatomic) LSCFunctionValue *setterFunc;

@end

@implementation LSCPropertyDescription

- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
                                 getter:(SEL)getter
                                 setter:(SEL)setter
{
    if (self = [super init])
    {
        _typeDescription = typeDescription;
        self.getter = getter;
        self.setter = setter;
    }
    return self;
}

- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
                             getterFunc:(LSCFunctionValue *)getterFunc
                             setterFunc:(LSCFunctionValue *)setterFunc
{
    if (self = [super init])
    {
        _typeDescription = typeDescription;
        self.getterFunc = getterFunc;
        self.setterFunc = setterFunc;
    }
    return self;
}

- (id<LSCValueType>)getValueWithInstance:(LSCInstance *)instance
                                 context:(LSCContext *)context
{
    if (self.getter)
    {
        NSMethodSignature *sign = [self.typeDescription.nativeType instanceMethodSignatureForSelector:self.getter];
        NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
        invocation.selector = self.getter;
        invocation.target = instance.object;
        
        Method m = class_getInstanceMethod(self.typeDescription.nativeType, invocation.selector);
        
        return [self.typeDescription _callMethod:m
                                      invocation:invocation
                                       arguments:nil
                                         context:context];
        
    }
    else if (self.getterFunc)
    {
        LSCInstanceValue *instValue = [LSCInstanceValue createValue:instance];
        return [self.getterFunc invokeWithArguments:@[instValue] context:context];
    }
    else
    {
        [context raiseExceptionWithMessage:@"The property cannot be read"];
    }
    
    return nil;
}

- (void)setValue:(id<LSCValueType>)value
    withInstance:(LSCInstance *)instance
         context:(LSCContext *)context
{
    if (self.setter)
    {
        NSMethodSignature *sign = [self.typeDescription.nativeType instanceMethodSignatureForSelector:self.setter];
        NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
        invocation.selector = self.setter;
        invocation.target = instance.object;
        
        Method m = class_getInstanceMethod(self.typeDescription.nativeType, invocation.selector);
        
        [self.typeDescription _callMethod:m
                               invocation:invocation
                                arguments:@[value]
                                  context:context];
    }
    else if (self.setterFunc)
    {
        LSCInstanceValue *instValue = [LSCInstanceValue createValue:instance];
        [self.setterFunc invokeWithArguments:@[instValue, value] context:context];
    }
    else
    {
        [context raiseExceptionWithMessage:@"The property is read-only"];
    }
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

@end
