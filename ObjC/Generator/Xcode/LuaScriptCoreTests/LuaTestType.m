//
//  LuaTestType.m
//  LuaScriptCoreTests
//
//  Created by 冯鸿杰 on 2020/9/4.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LuaTestType.h"

@implementation LuaTestType

- (instancetype)init
{
    if (self = [super init])
    {
        self.A = @"Hello World";
    }
    return self;
}

+ (void)callMethod
{
    NSLog(@"call void method");
}

+ (NSInteger)callMethodIntValue:(NSInteger)value
{
    NSLog(@"call integer method = %ld", value);
    
    return value;
}

+ (double)callMethod:(id)obj doubleValue:(double)value;
{
    NSLog(@"call double method = %f", value);
    return value;
}

+ (BOOL)callMethod:(id)obj booleanValue:(BOOL)value
{
    NSLog(@"call bool method = %d", value);
    return value;
}

+ (NSString *)callMethod:(id)obj stringValue:(NSString *)stringValue
{
    NSLog(@"call string method = %@", stringValue);
    return stringValue;
}

@end
