//
//  LSCModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/1.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LSCContext;

/**
 模块协议，用于扩展LSC功能
 */
@protocol LSCModule <NSObject>

/**
 获取模块标识，该标识必须唯一

 @return 模块标识
 */
+ (NSString *)moduleId;

/**
 注册模块时触发

 @param context 上下文对象
 */
+ (void)registerModuleWithContext:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
