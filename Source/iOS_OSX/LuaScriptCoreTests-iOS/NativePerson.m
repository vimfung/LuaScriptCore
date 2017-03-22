//
//  NativePerson.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/3/22.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "NativePerson.h"

@implementation NativePerson

- (void)speak:(NSString *)content
{
    NSLog(@"%@ speak:\"%@\"", self.name, content);
}

+ (void)printPersonName:(NativePerson *)person
{
    NSLog(@"person name = %@", person.name);
}

+ (NativePerson *)createPerson
{
    return [[NativePerson alloc] init];
}

@end
