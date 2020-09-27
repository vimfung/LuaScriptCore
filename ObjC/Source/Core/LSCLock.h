//
//  LSCLock.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/11.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 同步锁
 */
@interface LSCLock : NSObject


/**
 锁定
 */
- (void)lock;

/**
 解锁
 */
- (void)unlock;

@end

NS_ASSUME_NONNULL_END
