//
//  LSCExportPropertyDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/24.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportPropertyDescriptor.h"
#import "LSCFunction.h"
#import "LSCExportTypeDescriptor+Private.h"
#import "LSCValue.h"

@interface LSCExportPropertyDescriptor ()

/**
 属性名称
 */
@property (nonatomic, copy) NSString *name;

/**
 是否为原生属性
 */
@property (nonatomic) BOOL isNative;

/**
 Getter方法
 */
@property (nonatomic) SEL getterSelector;

/**
 Setter方法
 */
@property (nonatomic) SEL setterSelector;

/**
 Getter方法，在Lua中定义
 */
@property (nonatomic, strong) LSCFunction *getterFunction;

/**
 Setter方，在Lua中定义
 */
@property (nonatomic, strong) LSCFunction *setterFunction;

@end

@implementation LSCExportPropertyDescriptor

- (instancetype)initWithName:(NSString *)name
              getterSelector:(SEL)getterSelector
              setterSelector:(SEL)setterSelector
{
    if (self = [super init])
    {
        self.name = name;
        self.getterSelector = getterSelector;
        self.setterSelector = setterSelector;
        self.isNative = YES;
    }
    
    return self;
}

- (instancetype)initWithName:(NSString *)name
              getterFunction:(LSCFunction *)getterFunction
              setterFunction:(LSCFunction *)setterFunction
{
    if (self = [super init])
    {
        self.name = name;
        self.getterFunction = getterFunction;
        self.setterFunction = setterFunction;
        self.isNative = NO;
    }
    
    return self;
}

- (LSCValue *)invokeGetterWithInstance:(id)instance
                        typeDescriptor:(LSCExportTypeDescriptor *)typeDescriptor
{
    LSCValue *retValue = nil;
    if (self.isNative)
    {
        //为原生Getter
        if (self.getterSelector)
        {
            NSMethodSignature *sign = [typeDescriptor.nativeType instanceMethodSignatureForSelector:self.getterSelector];
            NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
            invocation.selector = self.getterSelector;

            retValue = [typeDescriptor _invokeMethodWithInstance:instance
                                                      invocation:invocation
                                                       arguments:nil];
        }
    }
    else
    {
        //为Lua Getter
        if (self.getterFunction)
        {
            retValue = [self.getterFunction invokeWithArguments:@[[LSCValue objectValue:instance]]];
        }
    }
    
    return retValue;
}

- (void)invokeSetterWithInstance:(id)instance
                  typeDescriptor:(LSCExportTypeDescriptor *)typeDescriptor
                           value:(LSCValue *)value
{
    if (self.isNative)
    {
        //为原生Setter
        if (self.setterSelector)
        {
            NSMethodSignature *sign = [typeDescriptor.nativeType instanceMethodSignatureForSelector:self.setterSelector];
            NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
            invocation.selector = self.setterSelector;
            
            //调用Setter方法
            [typeDescriptor _invokeMethodWithInstance:instance
                                           invocation:invocation
                                            arguments:@[[LSCValue objectValue:instance], value]];
        }
    }
    else
    {
        //Lua Setter
        if (self.setterFunction)
        {
            [self.setterFunction invokeWithArguments:@[[LSCValue objectValue:instance], value]];
        }
    }
}

@end
