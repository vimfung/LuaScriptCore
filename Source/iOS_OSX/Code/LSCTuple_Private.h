//
//  LSCTuple_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/17.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCTuple.h"

@class LSCContext;
@class LSCSession;

@interface LSCTuple ()

/**
 返回值集合
 */
@property (nonatomic, strong) NSMutableArray *returnValues;

@end
