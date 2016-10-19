//
//  LSCClass.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"
#import "LSCModule.h"

@class LSCClassInstance;
@class LSCContext;

/**
 *  Lua对象基类，为lua提供面向对象的基础，继承该类型用于衍生其他的类型
 */
@interface LSCObjectClass : LSCModule


/**
 上下文对象
 */
@property (nonatomic, weak, readonly) LSCContext *context;


/**
 初始化类实例

 @param context 上下文对象

 @return 类实例对象
 */
- (instancetype)initWithContext:(LSCContext *)context;

@end
