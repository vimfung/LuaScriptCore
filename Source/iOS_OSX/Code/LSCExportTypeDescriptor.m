//
//  LSCExportTypeDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportTypeDescriptor.h"
#import "LSCExportTypeDescriptor+Private.h"
#import "LSCValue.h"
#import "LSCExportMethodDescriptor.h"
#import "LSCExportPropertyDescriptor.h"
#import <objc/runtime.h>

@implementation LSCExportTypeDescriptor

+ (LSCExportTypeDescriptor *)objectTypeDescriptor
{
    static LSCExportTypeDescriptor *objectTypeDesc = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        objectTypeDesc = [[LSCExportTypeDescriptor alloc] init];
        objectTypeDesc.typeName = @"Object";
        objectTypeDesc.prototypeTypeName = [objectTypeDesc _prototypeClassNameWithTypeName:objectTypeDesc.typeName];
    });
    
    return objectTypeDesc;
}

- (instancetype)init
{
    if (self = [super init])
    {
        self.methodsMapping = [NSMutableDictionary dictionary];
    }
    return self;
}

- (instancetype)initWithTypeName:(NSString *)typeName
                      nativeType:(Class)nativeType
{
    if (self = [self init])
    {
        self.typeName = [typeName stringByReplacingOccurrencesOfString:@"." withString:@"_"];
        self.prototypeTypeName = [self _prototypeClassNameWithTypeName:self.typeName];
        self.nativeType = nativeType;
    }
    return self;
}

- (BOOL)subtypeOfType:(LSCExportTypeDescriptor *)typeDescriptor
{
    LSCExportTypeDescriptor *targetTypeDescriptor = self;
    while (targetTypeDescriptor)
    {
        if (targetTypeDescriptor == typeDescriptor)
        {
            return YES;
        }
        
        targetTypeDescriptor = targetTypeDescriptor.parentTypeDescriptor;
    }
    
    return NO;
}

- (LSCExportMethodDescriptor *)classMethodWithName:(NSString *)name arguments:(NSArray<LSCValue *> *)arguments
{
    return [self _methodWithName:name arguments:arguments isStatic:YES];
}


- (LSCExportMethodDescriptor *)instanceMethodWithName:(NSString *)name arguments:(NSArray<LSCValue *> *)arguments
{
    return [self _methodWithName:name arguments:arguments isStatic:NO];
}

#pragma mark - Private

/**
 获取原型类名称
 
 @param typeName 类型名称
 @return 原型类名称
 */
- (NSString *)_prototypeClassNameWithTypeName:(NSString *)typeName
{
    return [NSString stringWithFormat:@"_%@_PROTOTYPE_", typeName];
}

/**
 获取方法

 @param name 方法名称
 @param arguments 参数
 @param isStatic 是否为静态方法
 @return 方法描述
 */
- (LSCExportMethodDescriptor *)_methodWithName:(NSString *)name
                                     arguments:(NSArray<LSCValue *> *)arguments
                                      isStatic:(BOOL)isStatic
{
    
    NSArray<LSCExportMethodDescriptor *> *methods = nil;
    if (isStatic)
    {
        methods = self.classMethods[name];
    }
    else
    {
        methods = self.instanceMethods[name];
    }
    
    
    if (methods.count > 1)
    {
        //进行筛选
        __block LSCExportMethodDescriptor *targetMethod = nil;
        
        int startIndex = isStatic ? 0 : 1;
        if (arguments.count > startIndex)
        {
            //带参数
            NSMutableArray<NSString *> *signArr = [NSMutableArray array];
            NSMutableString *signStrRegexp = [NSMutableString string];
            for (int i = startIndex; i < arguments.count; i++)
            {
                LSCValue *value = arguments[i];
                switch (value.valueType)
                {
                    case LSCValueTypeNumber:
                        [signArr addObject:@"N"];
                        [signStrRegexp appendString:@"[fdcislqCISLQB@]"];
                        break;
                    case LSCValueTypeBoolean:
                        [signArr addObject:@"B"];
                        [signStrRegexp appendString:@"[BcislqCISLQfd@]"];
                        break;
                    case LSCValueTypeInteger:
                        [signArr addObject:@"I"];
                        [signStrRegexp appendString:@"[cislqCISLQfdB@]"];
                        break;
                    default:
                        [signArr addObject:@"O"];
                        [signStrRegexp appendString:@"@"];
                        break;
                }
            }
            
            NSString *luaMethodSignStr = [NSString stringWithFormat:@"%@_%@", name, [signArr componentsJoinedByString:@""]];
            targetMethod = self.methodsMapping[luaMethodSignStr];
            
            if (!targetMethod)
            {
                NSMutableArray<LSCExportMethodDescriptor *> *matchMethods = [NSMutableArray array];
                NSRegularExpression *regExp = [[NSRegularExpression alloc] initWithPattern:signStrRegexp options:0 error:nil];
                [methods enumerateObjectsUsingBlock:^(LSCExportMethodDescriptor * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                    
                    NSTextCheckingResult *result = [regExp firstMatchInString:obj.paramsSignature options:0 range:NSMakeRange(0, obj.paramsSignature.length)];
                    if (result)
                    {
                        [matchMethods addObject:obj];
                    }
                    
                }];
                
                if (matchMethods.count > 0)
                {
                    //选择最匹配的方法
                    //备选方法
                    __block LSCExportMethodDescriptor *alternateMethod = nil;
                    [matchMethods enumerateObjectsUsingBlock:^(LSCExportMethodDescriptor * _Nonnull methodDesc, NSUInteger idx, BOOL * _Nonnull stop) {
                        
                        BOOL hasMatch = YES;
                        BOOL hasAlternate = NO;
                        for (int i = 0; i < methodDesc.paramsSignature.length; i++)
                        {
                            if (i < signArr.count)
                            {
                                NSString *nativeSign = [methodDesc.paramsSignature substringWithRange:NSMakeRange(i, 1)];
                                NSString *luaSign = signArr[i];
                                
                                if ([luaSign isEqualToString:@"N"]
                                    && ![nativeSign isEqualToString:@"f"]
                                    && ![nativeSign isEqualToString:@"d"])
                                {
                                    //不匹配浮点类型，则进行是否为整型类型检测，同时如果匹配则视为备选方案
                                    hasAlternate = YES;
                                    luaSign = @"I";
                                }
                                
                                if ([luaSign isEqualToString:@"B"]
                                    && ![nativeSign isEqualToString:@"B"])
                                {
                                    hasMatch = NO;
                                    break;
                                }
                                
                                if ([luaSign isEqualToString:@"I"]
                                    && ![nativeSign isEqualToString:@"c"]
                                    && ![nativeSign isEqualToString:@"i"]
                                    && ![nativeSign isEqualToString:@"s"]
                                    && ![nativeSign isEqualToString:@"l"]
                                    && ![nativeSign isEqualToString:@"q"]
                                    && ![nativeSign isEqualToString:@"C"]
                                    && ![nativeSign isEqualToString:@"I"]
                                    && ![nativeSign isEqualToString:@"S"]
                                    && ![nativeSign isEqualToString:@"L"]
                                    && ![nativeSign isEqualToString:@"Q"])
                                {
                                    hasMatch = NO;
                                    break;
                                }
                                
                                if ([luaSign isEqualToString:@"O"]
                                    && ![nativeSign isEqualToString:@"@"])
                                {
                                    hasMatch = NO;
                                    break;
                                }
                            }
                        }
                        
                        if (hasMatch)
                        {
                            if (hasAlternate)
                            {
                                alternateMethod = methodDesc;
                            }
                            else
                            {
                                targetMethod = methodDesc;
                                *stop = YES;
                            }
                        }
                    }];
                    
                    if (!targetMethod)
                    {
                        if (alternateMethod)
                        {
                            //使用备选方法
                            targetMethod = alternateMethod;
                        }
                        else
                        {
                            //没有最匹配则使用第一个方法
                            targetMethod = matchMethods.firstObject;
                        }
                    }
                    
                    //设置方法映射
                    [self.methodsMapping setObject:targetMethod forKey:luaMethodSignStr];
                }
                
            }
        }
        else
        {
            //不带参数
            [methods enumerateObjectsUsingBlock:^(LSCExportMethodDescriptor * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                
                if ([obj.paramsSignature isEqualToString:@""])
                {
                    targetMethod = obj;
                    *stop = NO;
                }
                
            }];
        }
        
        return targetMethod;
    }
    else
    {
        return methods.firstObject;
    }

}

- (LSCValue *)_invokeMethodWithInstance:(id)instance
                             invocation:(NSInvocation *)invocation
                              arguments:(NSArray *)arguments
{
    //修复float类型在Invocation中会丢失问题，需要定义该结构体来提供给带float参数的方法。同时返回值处理也一样。
    typedef struct {float f;} LSCFloatStruct;
    
    //修正索引值，用于标识参数从哪个位置开始取值
    //其中实例方法第一个参数是实例对象自身，因此要从第二个参数开始取值，而类方法不存在该问题
    int fixedIndex = 0;
    Method m = NULL;
    if (instance)
    {
        //为实例方法
        [invocation setTarget:instance];
        fixedIndex = 1;
        m = class_getInstanceMethod(self.nativeType, invocation.selector);
    }
    else
    {
        //为类方法
        [invocation setTarget:self.nativeType];
        fixedIndex = 2;
        m = class_getClassMethod(self.nativeType, invocation.selector);
    }
    
    [invocation retainArguments];
    
    for (int i = 2; i < method_getNumberOfArguments(m); i++)
    {
        char *argType = method_copyArgumentType(m, i);
        
        LSCValue *value = nil;
        if (i - fixedIndex < arguments.count)
        {
            value = arguments[i - fixedIndex];
        }
        else
        {
            value = [LSCValue nilValue];
        }
        
        if (strcmp(argType, @encode(float)) == 0)
        {
            //浮点型数据
            LSCFloatStruct floatValue = {[value toDouble]};
            [invocation setArgument:&floatValue atIndex:i];
        }
        else if (strcmp(argType, @encode(double)) == 0)
        {
            //双精度浮点型
            double doubleValue = [value toDouble];
            [invocation setArgument:&doubleValue atIndex:i];
        }
        else if (strcmp(argType, @encode(BOOL)) == 0)
        {
            //布尔类型
            BOOL boolValue = [value toBoolean];
            [invocation setArgument:&boolValue atIndex:i];
        }
        else if (strcmp(argType, @encode(int)) == 0
                 || strcmp(argType, @encode(unsigned int)) == 0
                 || strcmp(argType, @encode(long)) == 0
                 || strcmp(argType, @encode(unsigned long)) == 0
                 || strcmp(argType, @encode(short)) == 0
                 || strcmp(argType, @encode(unsigned short)) == 0
                 || strcmp(argType, @encode(char)) == 0
                 || strcmp(argType, @encode(unsigned char)) == 0)
        {
            //整型
            NSInteger intValue = [value toDouble];
            [invocation setArgument:&intValue atIndex:i];
        }
        else if (strcmp(argType, @encode(id)) == 0)
        {
            //对象类型
            id obj = [value toObject];
            [invocation setArgument:&obj atIndex:i];
        }
        
        free(argType);
    }
    
    [invocation invoke];
    
    char *returnType = method_copyReturnType(m);
    
    LSCValue *retValue = nil;
    if (strcmp(returnType, @encode(id)) == 0)
    {
        //返回值为对象，添加__unsafe_unretained修饰用于修复ARC下retObj对象被释放问题。
        id __unsafe_unretained retObj = nil;
        [invocation getReturnValue:&retObj];
        
        retValue = [LSCValue objectValue:retObj];
    }
    else if (strcmp(returnType, @encode(BOOL)) == 0)
    {
        //fixed：修复在32位设备下，由于BOOL和char类型返回一样导致，无法正常识别BOOL值问题，目前处理方式将BOOL值判断提前。但会引起32位下char的返回得不到正确的判断。考虑char的使用频率没有BOOL高，故折中处理该问题。
        //B 布尔类型
        BOOL boolValue = NO;
        [invocation getReturnValue:&boolValue];
        retValue = [LSCValue booleanValue:boolValue];
    }
    else if (strcmp(returnType, @encode(int)) == 0
             || strcmp(returnType, @encode(unsigned int)) == 0
             || strcmp(returnType, @encode(long)) == 0
             || strcmp(returnType, @encode(unsigned long)) == 0
             || strcmp(returnType, @encode(short)) == 0
             || strcmp(returnType, @encode(unsigned short)) == 0
             || strcmp(returnType, @encode(char)) == 0
             || strcmp(returnType, @encode(unsigned char)) == 0)
    {
        // i 整型
        // I 无符号整型
        // q 长整型
        // Q 无符号长整型
        // S 无符号短整型
        // c 字符型
        // C 无符号字符型
        
        NSInteger intValue = 0;
        [invocation getReturnValue:&intValue];
        retValue = [LSCValue integerValue:intValue];
    }
    else if (strcmp(returnType, @encode(float)) == 0)
    {
        // f 浮点型，需要将值保存到floatStruct结构中传入给方法，否则会导致数据丢失
        LSCFloatStruct floatStruct = {0};
        [invocation getReturnValue:&floatStruct];
        retValue = [LSCValue numberValue:@(floatStruct.f)];
        
    }
    else if (strcmp(returnType, @encode(double)) == 0)
    {
        // d 双精度浮点型
        double doubleValue = 0.0;
        [invocation getReturnValue:&doubleValue];
        retValue = [LSCValue numberValue:@(doubleValue)];
    }
    else
    {
        //nil
        retValue = nil;
    }
    
    free(returnType);
    
    return retValue;
}

@end
