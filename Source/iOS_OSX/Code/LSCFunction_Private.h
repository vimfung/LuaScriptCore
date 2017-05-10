//
//  LSCFunction_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCFunction.h"
#import "lua.h"

@class LSCContext;

@interface LSCFunction ()

/**
 上下文对象
 */
@property (nonatomic, strong) LSCContext *context;


/**
 方法的本地索引
 */
@property (nonatomic, copy) NSString *index;

/**
 初始化Lua方法

 @param context 上下文对象
 @param index   参数索引

 @return Lua方法对象
 */
- (instancetype)initWithContext:(LSCContext *)context index:(NSInteger)index;

@end
