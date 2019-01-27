//
//  LSCTable+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/1/16.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "LSCTable.h"
#import "LSCContext_Private.h"

NS_ASSUME_NONNULL_BEGIN

@interface LSCTable ()

/**
 是否为数组
 */
@property (nonatomic, readonly) BOOL isArray;

/**
 上下文对象
 */
@property (nonatomic, strong) LSCContext *context;

/**
 连接标识
 */
@property (nonatomic, copy, readonly) NSString *linkId;

/**
 值对象
 */
@property (nonatomic, strong) id valueObject;

@end

NS_ASSUME_NONNULL_END
