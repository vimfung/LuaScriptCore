//
//  LSCTuple_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/17.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCTuple.h"

@class LSCContext;

@interface LSCTuple ()

/**
 返回值集合
 */
@property (nonatomic, strong) NSMutableArray *returnValues;

/**
 将元组入栈

 @param context 上下文对象
 */
- (void)pushWithContext:(LSCContext *)context;

@end
