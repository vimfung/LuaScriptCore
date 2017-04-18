//
//  NativePerson.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/3/22.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Person;

@interface NativePerson : NSObject

@property (nonatomic, copy) NSString *name;

- (void)speak:(NSString *)content;

+ (void)printPersonName:(NativePerson *)person;

+ (Person *)createPerson;

@end
