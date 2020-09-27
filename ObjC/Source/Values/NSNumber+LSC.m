//
//  NSNumber+LSC.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/6.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "NSNumber+LSC.h"
#import <objc/message.h>
#import <objc/runtime.h>

@implementation NSNumber (LSC)

- (BOOL)isBoolNumber
{
    return [self isKindOfClass:@YES.class];
}

@end
