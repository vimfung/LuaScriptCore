//
//  LSCObjectProxy.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/3/20.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCModule.h"

@class LSCContext;

/**
 原生类型导入到Lua时使用
 */
@interface LSCClassImport : LSCModule

/**
 设置可以导入的原生类型，默认为nil，表示无法导入任何类型。

 @param classes 类型列表
 @param context 上下文对象
 */
+ (void)setInculdesClasses:(NSArray<Class> *)classes withContext:(LSCContext *)context;

@end
