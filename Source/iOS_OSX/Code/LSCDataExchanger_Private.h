//
//  LSCDataExchanger_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/7/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCDataExchanger.h"
#import "LSCContext_Private.h"
#import "lauxlib.h"
#import "lua.h"
#import "luaconf.h"
#import "lualib.h"

@interface LSCDataExchanger ()

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

@end
