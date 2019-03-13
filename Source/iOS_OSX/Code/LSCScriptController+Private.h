//
//  LSCRunScriptConfig+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/3/12.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "LSCScriptController.h"

NS_ASSUME_NONNULL_BEGIN

@interface LSCScriptController ()

/**
 脚本超时设置(单位：秒)，超过时间则会强制退出脚本执行，默认值为0，表示没有超时限制
 */
@property (nonatomic) NSInteger timeout;

/**
 是否强制结束脚本，默认为NO
 */
@property (nonatomic) BOOL isForceExit;

/**
 开始时间
 */
@property (nonatomic) NSTimeInterval startTime;

@end

NS_ASSUME_NONNULL_END
