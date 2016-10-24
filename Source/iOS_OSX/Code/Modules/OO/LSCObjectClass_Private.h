//
//  LSCObjectClass_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/20.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCObjectClass.h"

@interface LSCObjectClass ()

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

/**
 查找实例对应的lua引用

 @param instance 实例对象

 @return lua的实例引用
 */
+ (void**)_findLuaRef:(LSCObjectClass *)instance;

@end
