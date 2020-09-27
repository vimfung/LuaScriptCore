//
//  LSCFunctionValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

/**
 *  方法处理器
 *
 *  @param arguments 参数列表
 *
 *  @return 返回值
 */
typedef id<LSCValueType> (^LSCFunctionHandler) (NSArray<id<LSCValueType>> *arguments);

@class LSCContext;


NS_ASSUME_NONNULL_BEGIN

/**
 方法类型
 */
@interface LSCFunctionValue : NSObject <LSCValueType>

/**
 是否为原生方法，YES 原生方法，否则为Lua方法
 */
@property (nonatomic, readonly) BOOL nativeFunction;

/**
 初始化对象
 
 @param handler 方法处理器
 @return 对象实例
 */
- (instancetype)initWithHandler:(LSCFunctionHandler)handler;

/**
 调用方法

 @param arguments 参数列表
 @param context 上下文对象
 @return 返回值
 */
- (id<LSCValueType>)invokeWithArguments:(nullable NSArray<id<LSCValueType>> *)arguments
                                context:(nonnull LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
