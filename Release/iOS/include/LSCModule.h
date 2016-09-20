//
//  LSCModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"

/**
 *  Lua模块，用于扩展时使用
 */
@interface LSCModule : NSObject

/**
 *  模块名称
 */
@property (nonatomic, copy, readonly) NSString *name;

/**
 *  初始化模块
 *
 *  @param name 模块名称，注：每个模块的名字都必须唯一，否则会导致模块无法注册
 *
 *  @return 模块对象
 */
- (instancetype)initWithName:(NSString *)name;

/**
 *  注册模块方法
 *
 *  @param methodName 方法名称
 *  @param block      方法处理器
 */
- (void)registerMethodWithName:(NSString *)methodName
                         block:(LSCFunctionHandler)block;

@end
