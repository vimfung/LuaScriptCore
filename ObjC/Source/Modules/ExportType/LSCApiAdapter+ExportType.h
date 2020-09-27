//
//  LSCApiAdapter+ExportType.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/2.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCApiAdapter.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCTypeDescription;
@class LSCInstance;

@interface LSCApiAdapter (ExportType)

/**
 获取类型
 
 @param stackIndex 栈索引
 @param context 上下文对象
 @return 类型描述
 */
- (LSCTypeDescription *)getTypeDescriptionWithStackIndex:(int)stackIndex
                                        context:(LSCContext *)context;

/**
 入栈类型

 @param typeDescription 类型描述
 @param context 上下文
 */
- (void)pushType:(LSCTypeDescription *)typeDescription
         context:(LSCContext *)context;

/**
 入栈实例

 @param instance 实例对象
 @param context 上下文
 */
- (void)pushInstance:(LSCInstance *)instance
             context:(LSCContext *)context;

/**
 观察全局变量表中取得
 
 @param context 上下文
 */
- (void)watchGlobalWithContext:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
