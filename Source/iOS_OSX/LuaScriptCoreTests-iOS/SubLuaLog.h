//
//  SubLuaLog.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LuaLog.h"

@interface SubLuaLog : LuaLog

@property (nonatomic, copy) NSString *name;

- (void)printName;

@end
