//
//  TestModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCExportType.h"

@interface TestModule : NSObject <LSCExportType>

+ (NSString *)test;

+ (NSString *)testWithMsg:(NSString *)msg;

@end
