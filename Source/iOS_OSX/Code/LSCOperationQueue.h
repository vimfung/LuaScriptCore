//
//  LSCOperationQueue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/6/28.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 操作队列
 */
@interface LSCOperationQueue : NSObject

/**
 执行操作

 @param block 操作内容
 */
- (void)performAction:(void (^)(void))block;


@end
