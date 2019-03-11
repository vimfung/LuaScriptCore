//
//  LSCError.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/3/11.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LSCSession;

/**
 异常信息
 */
@interface LSCError : NSObject

/**
 消息
 */
@property (nonatomic, copy, readonly) NSString *message;

/**
 会话
 */
@property (nonatomic, weak, readonly) LSCSession *session;


/**
 初始化会话

 @param session 会话对象
 @param message 消息
 @return 异常信息
 */
- (instancetype)initWithSession:(LSCSession *)session
                        message:(NSString *)message;

@end

NS_ASSUME_NONNULL_END
