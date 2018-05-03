//
//  Person.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCExportType.h"
#import "LSCExportTypeAnnotation.h"

@class NativePerson;
@class LSCTuple;
@class LSCFunction;

@interface Person : NSObject <LSCExportType>

@property (nonatomic, copy) NSString *name;

- (void)speak:(NSString *)content;
- (void)speakWithAge:(BOOL)age;
- (LSCTuple *)test;

+ (void)printPersonName:(Person *)person;

+ (Person *)createPerson;

+ (NativePerson *)createNativePerson;

+ (void)testFuncRelease:(LSCFunction *)func;

+ (BOOL)returnBoolean;

+ (char)returnChar;

@end
