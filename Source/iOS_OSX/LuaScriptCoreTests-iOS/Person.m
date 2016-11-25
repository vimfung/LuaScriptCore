//
//  Person.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "Person.h"

@implementation Person

- (void)speak:(NSString *)content
{
    NSLog(@"%@ speak:\"%@\"", self.name, content);
}

+ (void)printPersonName:(Person *)person
{
    NSLog(@"person name = %@", person.name);
}

+ (Person *)createPerson
{
    return [[Person alloc] init];
}

@end
