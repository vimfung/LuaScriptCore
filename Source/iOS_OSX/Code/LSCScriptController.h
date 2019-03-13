//
//  LSCRunScriptConfig.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/3/12.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 执行脚本设置
 */
@interface LSCScriptController : NSObject

/**
 设置脚本超时时间

 @param timeout 超时时间（单位：秒），如果传入0则表示不限制执行时间
 */
- (void)setTimeout:(NSInteger)timeout;

/**
 强制退出执行脚本
 */
- (void)forceExit;

@end

NS_ASSUME_NONNULL_END
