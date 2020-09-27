//
//  LSCFunctionValue+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/14.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCFunctionValue.h"

NS_ASSUME_NONNULL_BEGIN

@interface LSCFunctionValue ()

/**
 方法处理器
 */
@property (nonatomic, strong) LSCFunctionHandler handler;

/**
 Lua方法标识
 */
@property (nonatomic, copy) NSString *functionId;

/**
 如果function由Lua回传到原生层，则该属性会记录function所属的上下文对象
 */
@property (nonatomic, weak) LSCContext *ownerContext;

@end

NS_ASSUME_NONNULL_END
