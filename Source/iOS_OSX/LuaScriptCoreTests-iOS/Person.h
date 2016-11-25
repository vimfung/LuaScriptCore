//
//  Person.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCObjectClass.h"

@interface Person : LSCObjectClass

@property (nonatomic, copy) NSString *name;

- (void)speak:(NSString *)content;

+ (void)printPersonName:(Person *)person;

+ (Person *)createPerson;

@end
