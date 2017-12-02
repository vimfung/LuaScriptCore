//
//  LuaLog.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/6.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCExportType.h"

@interface LuaLog : NSObject <LSCExportType>

+ (void)writeLog:(NSString *)msg;

@end
