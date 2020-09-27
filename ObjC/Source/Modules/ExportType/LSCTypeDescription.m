//
//  LSCClassDescription.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCTypeDescription+Private.h"
#import "LSCExportType.h"
#import "LSCApiAdapter.h"
#import "LSCApiAdapter+ExportType.h"
#import "LSCContext+ExportType.h"
#import "LSCException.h"
#import "LSCExportTypeModule.h"
#import "LSCExportTypeRule.h"
#import "LSCMethodDescription.h"
#import "LSCPropertyDescription.h"
#import "LSCNumberValue.h"
#import "LSCBooleanValue.h"
#import "LSCInstance+Private.h"
#import "LSCValue.h"

#import <objc/runtime.h>

@implementation LSCTypeDescription

+ (LSCTypeDescription *)typeDescriptionByClass:(Class)aClass
{
    LSCTypeDescription *typeDescription = nil;
    if ([aClass conformsToProtocol:@protocol(LSCExportType)])
    {
        typeDescription = [[LSCTypeDescription alloc] _initWithNativeType:aClass];
    }
    
    return typeDescription;
}

+ (LSCTypeDescription *)objectTypeDescription
{
    static LSCTypeDescription *objectTypeDescription = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        objectTypeDescription = [[LSCTypeDescription alloc] _initObjectTypeDescription];
    });
    
    return objectTypeDescription;
}

- (id<LSCValueType>)callMethodWithInstance:(LSCInstance *)instance
                                      name:(NSString *)name
                                 arguments:(NSArray<id<LSCValueType>> *)arguments
                                   context:(LSCContext *)context
{
    LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
    if ([name isEqualToString:rule.constructMethodName])
    {
        //无法直接调用构造方法
        [context raiseExceptionWithMessage:@"Cannot directly call the constructor"];
        return nil;
    }
    
    id<LSCValueType> resultValue = nil;
    if (!self.nativeType || [instance.object isKindOfClass:[self.nativeType class]])
    {
        LSCMethodDescription *methodDesc = [self _findMehtodWithName:name
                                                           arguments:arguments
                                                             methods:self.instanceMethods];
        if (methodDesc)
        {
            resultValue = [methodDesc invokeWithTarget:instance
                                             arguments:arguments
                                               context:context];
        }
        else
        {
            NSString *msg = [NSString stringWithFormat:@"`%@` is an invalid method or the incoming parameters do not match", name];
            [context raiseExceptionWithMessage:msg];
        }
    }
    
    return resultValue;
}

- (id<LSCValueType>)callMethodWithName:(NSString *)name
                             arguments:(NSArray<id<LSCValueType>> *)arguments
                               context:(nonnull LSCContext *)context
{
    LSCMethodDescription *methodDesc = [self _findMehtodWithName:name
                                                       arguments:arguments
                                                         methods:self.classMethods];
    if (methodDesc)
    {
        return [methodDesc invokeWithTarget:self arguments:arguments context:context];
    }
    else
    {
        NSString *msg = [NSString stringWithFormat:@"`%@` is an invalid method or the incoming parameters do not match", name];
        [context raiseExceptionWithMessage:msg];
    }
    
    return nil;
}

- (void)registerPropertyWithName:(NSString *)name
                      getterFunc:(LSCFunctionValue *)getterFunc
                      setterFunc:(LSCFunctionValue *)setterFunc
                         context:(LSCContext *)context
{
    if (!self.properties[name])
    {
        LSCPropertyDescription *propDesc = [[LSCPropertyDescription alloc] initWithTypeDescription:self getterFunc:getterFunc setterFunc:setterFunc];
        [self.properties setObject:propDesc forKey:name];
    }
    else
    {
        NSString *msg = [NSString stringWithFormat:@"`%@` property already exists", name];
        [context raiseExceptionWithMessage:msg];
    }
}

- (id<LSCValueType>)getPropertyWithInstance:(LSCInstance *)instance
                                       name:(NSString *)name
                                    context:(LSCContext *)context
{
    //查找属性描述
    LSCPropertyDescription *propDesc = [self _findPrpoertyWithName:name];
    if (propDesc)
    {
        return [propDesc getValueWithInstance:instance context:context];
    }
    
    return nil;
}

- (void)setPropertyWithInstance:(LSCInstance *)instance
                           name:(NSString *)name
                          value:(id<LSCValueType>)value
                        context:(LSCContext *)context
{
    LSCPropertyDescription *propDesc = [self _findPrpoertyWithName:name];
    if (propDesc)
    {
        [propDesc setValue:value withInstance:instance context:context];
    }
}

- (BOOL)existsPropertyeWithName:(NSString *)name
{
    LSCPropertyDescription *propDesc = [self _findPrpoertyWithName:name];
    return propDesc ? YES : NO;
}

- (LSCInstance *)constructInstanceWithInstanceId:(NSString *)instanceId
                                       arguments:(NSArray<id<LSCValueType>> *)arguments
                                         context:(LSCContext *)context
{
    
    if (self.nativeType)
    {
        id target = [self.nativeType alloc];
        
        //查找构造方法
        LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
        LSCMethodDescription *constructMethod = [self _findMehtodWithName:rule.constructMethodName
                                                                arguments:arguments
                                                                  methods:self.instanceMethods];
        if (!constructMethod)
        {
            //没有找到匹配的构造方法，则直接调用init方法
            target = [target init];
        }
        else
        {
            id<LSCValueType> retValue = [constructMethod invokeWithTarget:target
                                                                arguments:arguments
                                                                  context:context];
            target = retValue.rawValue;
        }
        
        if (target)
        {
            return [[LSCInstance alloc] _initWithInstanceId:instanceId
                                            typeDescription:self
                                                     object:target
                                                    context:context];
        }
        else
        {
            return nil;
        }
    }
    else
    {
        //Object类型无原生类型，则传入nil
        return [[LSCInstance alloc] _initWithInstanceId:instanceId
                                        typeDescription:self
                                                 object:nil
                                                context:context];
    }
}

- (LSCTypeDescription *)subtypeWithName:(NSString *)typeName
{
    LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
    
    LSCTypeDescription *subtypeDesc = [[LSCTypeDescription alloc] _init];
    subtypeDesc.nativeType = self.nativeType;
    subtypeDesc.typeName = typeName;
    subtypeDesc.prototypeName = [rule prototypeNameByTypeName:subtypeDesc.typeName];
    subtypeDesc.parentTypeDescription = self;
    
    return subtypeDesc;
}

- (BOOL)subtypeOfType:(LSCTypeDescription *)typeDescription
{
    LSCTypeDescription *targetTypeDescription = self;
    while (targetTypeDescription)
    {
        if (targetTypeDescription == typeDescription)
        {
            return YES;
        }
        
        targetTypeDescription = targetTypeDescription.parentTypeDescription;
    }
    
    return NO;
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"[%@ type]", self.typeName];
}

#pragma mark - Private

- (instancetype)_init
{
    if (self = [super init])
    {
        self.classMethods = [NSMutableDictionary dictionary];
        self.instanceMethods = [NSMutableDictionary dictionary];
        self.properties = [NSMutableDictionary dictionary];
    }
    return self;
}

- (instancetype)_initWithNativeType:(Class<LSCExportType>)nativeType
{
    if (self = [self _init])
    {
        LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
        
        self.nativeType = nativeType;
        self.typeName = [rule typeNameByClass:nativeType];
        self.prototypeName = [rule prototypeNameByTypeName:self.typeName];
        
        //获取需要导出的方法和属性
        [self _getExportClassMethodsWithTypeDescription:self];
        [self _getExportInstanceMethods];
        [self _getExportProperties];
        
    }
    return self;
}

- (instancetype)_initObjectTypeDescription
{
    if (self = [self _init])
    {
        LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
        
        self.typeName = @"Object";
        self.prototypeName = [rule prototypeNameByTypeName:self.typeName];
    }
    return self;
}


/**
 获取导出的类方法

 @param typeDescription 需要导出的类型
 */
- (void)_getExportClassMethodsWithTypeDescription:(LSCTypeDescription *)typeDescription
{
    LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
    if (typeDescription.nativeType)
    {
        //获取类方法
        Class metaType = objc_getMetaClass(NSStringFromClass(typeDescription.nativeType).UTF8String);
        
        unsigned int methodCount = 0;
        Method *methods = class_copyMethodList(metaType, &methodCount);
        for (const Method *m = methods; m < methods + methodCount; m ++)
        {
            SEL selector = method_getName(*m);
            if ([rule filterClassMethod:*m selector:selector class:typeDescription.nativeType])
            {
                continue;
            }
            
            NSString *methodName = [rule methodNameBySelector:selector];

            NSMutableArray<LSCMethodDescription *> *methodList = self.classMethods[methodName];
            if (!methodList)
            {
                methodList = [NSMutableArray array];
                [self.classMethods setObject:methodList forKey:methodName];
            }
            
            //判断参数签名是否存在相同
            __block BOOL hasExists = NO;
            NSString *paramsSign = [self _getParametersSignWithMethod:*m];
            [methodList enumerateObjectsUsingBlock:^(LSCMethodDescription * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
               
                if ([obj.paramsSignature isEqualToString:paramsSign])
                {
                    hasExists = YES;
                    *stop = YES;
                }
                
            }];
            
            if (!hasExists)
            {
                //获取返回类型
                char *returnType = method_copyReturnType(*m);
                NSString *returnTypeSign = [[NSString alloc] initWithUTF8String:returnType];
                free(returnType);
                
                //创建方法描述
                NSMethodSignature *sign = [typeDescription.nativeType methodSignatureForSelector:selector];
                LSCMethodDescription *methodDescription = [[LSCMethodDescription alloc] initWithTypeDescription:typeDescription selector:selector methodSignature:sign paramsSignature:paramsSign resultSignature:returnTypeSign];
                
                //添加到方法列表中
                [methodList addObject:methodDescription];
            }
            
        }
        free(methods);
    }
    
    //导出父级方法
    LSCTypeDescription *parentTypeDescriptor = typeDescription.parentTypeDescription;
    if (parentTypeDescriptor)
    {
        [self _getExportClassMethodsWithTypeDescription:parentTypeDescriptor];
    }
}

- (void)_getExportInstanceMethods
{
    LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
    if (self.nativeType)
    {
        unsigned int methodCount = 0;
        Method *methods = class_copyMethodList(self.nativeType, &methodCount);
        for (const Method *m = methods; m < methods + methodCount; m ++)
        {
            SEL selector = method_getName(*m);
            if ([rule filterInstanceMethod:*m selector:selector class:self.nativeType])
            {
                continue;
            }
            
            NSString *methodName = [rule methodNameBySelector:selector];
            NSMutableArray<LSCMethodDescription *> *methodList = self.instanceMethods[methodName];
            if (!methodList)
            {
                methodList = [NSMutableArray array];
                [self.instanceMethods setObject:methodList forKey:methodName];
            }
            
            //判断参数签名是否存在相同
            __block BOOL hasExists = NO;
            NSString *paramsSign = [self _getParametersSignWithMethod:*m];
            [methodList enumerateObjectsUsingBlock:^(LSCMethodDescription * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                
                if ([obj.paramsSignature isEqualToString:paramsSign])
                {
                    hasExists = YES;
                    *stop = YES;
                }
                
            }];
            
            if (!hasExists)
            {
                //获取返回类型
                char *returnType = method_copyReturnType(*m);
                NSString *returnTypeSign = [[NSString alloc] initWithUTF8String:returnType];
                free(returnType);
                
                //创建方法描述
                NSMethodSignature *sign = [self.nativeType methodSignatureForSelector:selector];
                LSCMethodDescription *methodDescription = [[LSCMethodDescription alloc] initWithTypeDescription:self selector:selector methodSignature:sign paramsSignature:paramsSign resultSignature:returnTypeSign];
                
                //添加到方法列表中
                [methodList addObject:methodDescription];
            }
        }
        free(methods);
    }
}

- (void)_getExportProperties
{
    LSCExportTypeRule *rule = [LSCExportTypeRule defaultRule];
    if (self.nativeType)
    {
        uint count = 0;
        objc_property_t *properties = class_copyPropertyList(self.nativeType, &count);
        for (int i = 0; i < count; i++)
        {
            objc_property_t property = *(properties + i);
            if ([rule filterProperty:property class:self.nativeType])
            {
                continue;
            }
            
            NSString *propertyName = [NSString stringWithUTF8String:property_getName(property)];
            
            //获取属性特性
            BOOL readonly = NO;
            NSString *getterName = nil;
            NSString *setterName = nil;
            uint attrCount = 0;
            objc_property_attribute_t *attrs = property_copyAttributeList(property, &attrCount);
            for (int j = 0; j < attrCount; j++)
            {
                objc_property_attribute_t attr = *(attrs + j);
                if (strcmp(attr.name, "G") == 0)
                {
                    getterName = [NSString stringWithUTF8String:attr.value];
                }
                else if (strcmp(attr.name, "S") == 0)
                {
                    //Setter
                    setterName = [NSString stringWithUTF8String:attr.value];
                }
                else if (strcmp(attr.name, "R") == 0)
                {
                    //只读属性
                    readonly = YES;
                }
            }
            free(attrs);
            
            //没有获得getter和setter名字时，则表示使用的是默认名称
            if (!getterName)
            {
                getterName = propertyName;
            }
            if (!setterName)
            {
                setterName = [NSString stringWithFormat:@"set%@%@:",
                              [propertyName.capitalizedString substringToIndex:1],
                              [propertyName substringFromIndex:1]];
            }
            
            //生成属性
            SEL getter = NSSelectorFromString(getterName);
            SEL setter = readonly ? nil : NSSelectorFromString(setterName);
            LSCPropertyDescription *propDesc = [[LSCPropertyDescription alloc] initWithTypeDescription:self getter:getter setter:setter];
            [self.properties setObject:propDesc forKey:propertyName];
        }
        
        free(properties);
    }
}

- (NSString *)_getParametersSignWithMethod:(Method)method
{
    NSMutableString *signStr = [NSMutableString string];
    int argCount = method_getNumberOfArguments(method);
    for (int i = 2; i < argCount; i++)
    {
        char s[256] = {0};
        method_getArgumentType(method, i, s, 256);
        [signStr appendString:[NSString stringWithUTF8String:s]];
    }
    
    return signStr;
}


/**
 查找方法

 @param name 方法名称
 @param arguments 参数列表
 @return 方法
 */
- (LSCMethodDescription *)_findMehtodWithName:(NSString *)name
                                    arguments:(NSArray<id<LSCValueType>> *)arguments
                                      methods:(LSCMethodMap *)methods
{
     __block LSCMethodDescription *targetMethod = nil;
    if (name)
    {
        NSArray<LSCMethodDescription *> *methods = self.classMethods[name];
        if (methods && methods.count > 0)
        {
            if (arguments.count > 0)
            {
                //带参数
                __block NSInteger maxWeight = 0;
                [methods enumerateObjectsUsingBlock:^(LSCMethodDescription * _Nonnull m, NSUInteger idx, BOOL * _Nonnull stop) {
                   
                    NSString *paramSign = m.paramsSignature;
                    
                    __block NSInteger weight = 0;
                    
                    //方法的参数必须要多于调用参数，否则表示传入
                    if (paramSign.length >= arguments.count)
                    {
                        if (paramSign.length == arguments.count)
                        {
                            //参数数量相同时，需要增加权重
                            weight += 1;
                        }
                        
                        __block BOOL hasMatch = YES;
                        [arguments enumerateObjectsUsingBlock:^(id<LSCValueType>  _Nonnull arg, NSUInteger idx, BOOL * _Nonnull stop) {
                            
                            unichar sign = [paramSign characterAtIndex:idx];
                            if ([arg isKindOfClass:[LSCNumberValue class]])
                            {
                                //从Lua中传入的Number类型都以double类型进行解析
                                //所以当参数签名为double时权重较大，其次到float，再到其他类型
                                if (sign == 'd')
                                {
                                    weight += 3;
                                }
                                else if (sign == 'f')
                                {
                                    weight += 2;
                                }
                                else
                                {
                                    weight += 1;
                                }
                            }
                            else if ([arg isKindOfClass:[LSCBooleanValue class]])
                            {
                                if (sign == 'B')
                                {
                                    weight += 2;
                                }
                                else
                                {
                                    weight += 1;
                                }
                            }
                            else
                            {
                                if (sign == '@')
                                {
                                    weight += 2;
                                }
                                else
                                {
                                    //不匹配
                                    hasMatch = NO;
                                    *stop = YES;
                                }
                            }
                            
                        }];
                        
                        if (hasMatch && weight > maxWeight)
                        {
                            targetMethod = m;
                            maxWeight = weight;
                        }
                    }
                    
                }];
            }
            else
            {
                //不带参数
                [methods enumerateObjectsUsingBlock:^(LSCMethodDescription * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                    
                    if ([obj.paramsSignature isEqualToString:@""])
                    {
                        targetMethod = obj;
                        *stop = YES;
                    }
                    
                }];
            }
        }
    }
    
    return targetMethod;
}

- (LSCPropertyDescription *)_findPrpoertyWithName:(NSString *)name
{
    LSCPropertyDescription *propDesc = nil;
    LSCTypeDescription *typeDesc = self;
    while (typeDesc)
    {
        propDesc = typeDesc.properties[name];
        if (propDesc)
        {
            break;
        }
        typeDesc = typeDesc.parentTypeDescription;
    }
    
    return propDesc;
}

- (id<LSCValueType>)_callMethod:(Method)method
                     invocation:(NSInvocation *)invocation
                      arguments:(NSArray<id<LSCValueType>> *)arguments
                        context:(LSCContext *)context
{
    //修复float类型在Invocation中会丢失问题，需要定义该结构体来提供给带float参数的方法。同时返回值处理也一样。
    typedef struct {float f;} LSCFloatStruct;
    
    if (invocation)
    {
        [invocation retainArguments];
        
        //填充参数
        for (int i = 2; i < method_getNumberOfArguments(method); i++)
        {
            char *argType = method_copyArgumentType(method, i);
            NSInteger argIndex = i - 2;
            
            id<LSCValueType> arg = nil;
            if (argIndex < arguments.count)
            {
                arg = arguments[argIndex];
            }
            else
            {
                arg = [LSCValue createValue:nil];
            }
            
            if (strcmp(argType, @encode(float)) == 0)
            {
                //浮点型数据
                LSCFloatStruct floatValue = {[arg.rawValue doubleValue]};
                [invocation setArgument:&floatValue atIndex:i];
            }
            else if (strcmp(argType, @encode(double)) == 0)
            {
                //双精度浮点
                double doubleValue = [arg.rawValue doubleValue];
                [invocation setArgument:&doubleValue atIndex:i];
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
                NSInteger intValue = [arg.rawValue integerValue];
                [invocation setArgument:&intValue atIndex:i];
            }
            else if (strcmp(argType, @encode(BOOL)) == 0)
            {
                //布尔
                BOOL boolValue = [arg.rawValue boolValue];
                [invocation setArgument:&boolValue atIndex:i];
            }
            else if (strcmp(argType, @encode(id)) == 0)
            {
                id obj = arg.rawValue;
                if ([obj isKindOfClass:[NSNull class]])
                {
                    obj = nil;
                }
                
                [invocation setArgument:&obj atIndex:i];
            }
            
            free(argType);
        }
        
        [invocation invoke];
        
        //处理返回值
        char *returnType = method_copyReturnType(method);
        
        id result = nil;
        if (strcmp(returnType, @encode(id)) == 0)
        {
            //返回值为对象，添加__unsafe_unretained修饰用于修复ARC下retObj对象被释放问题。
            id __unsafe_unretained retObj = nil;
            [invocation getReturnValue:&retObj];
            
            result = retObj;
        }
        else if (strcmp(returnType, @encode(BOOL)) == 0)
        {
            //fixed：修复在32位设备下，由于BOOL和char类型返回一样导致，无法正常识别BOOL值问题，目前处理方式将BOOL值判断提前。但会引起32位下char的返回得不到正确的判断。考虑char的使用频率没有BOOL高，故折中处理该问题。
            //B 布尔类型
            BOOL boolValue = NO;
            [invocation getReturnValue:&boolValue];
            
            result = @(boolValue);
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
            result = @(intValue);
        }
        else if (strcmp(returnType, @encode(float)) == 0)
        {
            // f 浮点型，需要将值保存到floatStruct结构中传入给方法，否则会导致数据丢失
            LSCFloatStruct floatStruct = {0};
            [invocation getReturnValue:&floatStruct];
            result = @(floatStruct.f);
        }
        else if (strcmp(returnType, @encode(double)) == 0)
        {
            // d 双精度浮点型
            double doubleValue = 0.0;
            [invocation getReturnValue:&doubleValue];
            result = @(doubleValue);
        }
        
        free(returnType);
        
        return [LSCValue createValue:result];
    }
    
    return nil;
}

@end
