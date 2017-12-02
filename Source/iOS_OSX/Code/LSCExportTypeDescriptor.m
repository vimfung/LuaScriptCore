//
//  LSCExportTypeDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportTypeDescriptor.h"
#import "LSCValue.h"
#import "LSCExportMethodDescriptor.h"
#import "LSCExportPropertyDescriptor.h"

@interface LSCExportTypeDescriptor ()

/**
 类型名称
 */
@property (nonatomic, copy) NSString *typeName;

/**
 原型类型名称
 */
@property (nonatomic, copy) NSString *prototypeTypeName;

/**
 原生类型
 */
@property (nonatomic) Class nativeType;

/**
 方法映射表
 */
@property (nonatomic, strong) NSMutableDictionary<NSString *,LSCExportMethodDescriptor *> *methodsMapping;

@end

@implementation LSCExportTypeDescriptor

+ (LSCExportTypeDescriptor *)objectTypeDescriptor
{
    LSCExportTypeDescriptor *objectTypeDesc = [[LSCExportTypeDescriptor alloc] init];
    objectTypeDesc.typeName = @"Object";
    objectTypeDesc.prototypeTypeName = [objectTypeDesc _prototypeClassNameWithTypeName:objectTypeDesc.typeName];
    
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
        self.typeName = typeName;
        self.prototypeTypeName = [self _prototypeClassNameWithTypeName:typeName];
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
                    
                    NSTextCheckingResult *result = [regExp firstMatchInString:obj.methodSignature options:0 range:NSMakeRange(0, obj.methodSignature.length)];
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
                        for (int i = 0; i < methodDesc.methodSignature.length; i++)
                        {
                            if (i < signArr.count)
                            {
                                NSString *nativeSign = [methodDesc.methodSignature substringWithRange:NSMakeRange(i, 1)];
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
                
                if ([obj.methodSignature isEqualToString:@""])
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

@end
