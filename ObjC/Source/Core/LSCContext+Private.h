//
//  LSCContext+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/1.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCContext.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCMainState;

@interface LSCContext ()

/**
 主状态
 */
@property (nonatomic, strong) LSCMainState *mainState;

/**
 *  异常捕获方法
 */
@property (nonatomic, strong, nullable) LSCFunctionValue *exceptionFunction;

/**
 模块集合
 Key - 模块ID
 Value - 模块
 */
@property (nonatomic, strong) NSMutableDictionary<NSString *, Class<LSCModule>> *modules;

/**
 模块自定义数据
 Key - 模块ID
 Value - 数据集合
 */
@property (nonatomic, strong) NSMutableDictionary<NSString *, NSMutableDictionary*> *moduleUserdatas;


/**
 调用垃圾回收标识，YES时表示需要进行垃圾回收
 */
@property (nonatomic) BOOL gcFlag;

@end

NS_ASSUME_NONNULL_END
