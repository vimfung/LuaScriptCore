//
//  LogModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"

@interface LogModule : LSCModule

+ (void)writeLog:(NSString *)m;

@end
