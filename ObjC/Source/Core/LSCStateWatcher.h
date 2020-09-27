//
//  LSCStateWatcher.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/25.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 观察事件
 
 - LSCWatchEventCall: 调用时触发
 - LSCWatchEventReturn: 返回前触发
 - LSCWatchEventLine: 每行触发
 - LSCWatchEventCount: 计数触发
 */
typedef NS_ENUM(NSUInteger, LSCStateWatcherEvents) {
    LSCStateWatcherEventCall = 1 << 0,
    LSCStateWatcherEventReturn = 1 << 1,
    LSCStateWatcherEventLine = 1 << 2,
    LSCStateWatcherEventCount = 1 << 3,
};

@class LSCContext;
@class LSCMainState;
@class LSCState;
@class LSCStateWatcher;


/**
 观察事件处理器

 @param watcher 观察器
 @param context 上下文
 @param mainState 主状态
 @param curState 当前状态
 */
typedef void(^LSCWatchEventHandler)(LSCStateWatcher *watcher,
                                    LSCContext *context,
                                    LSCMainState *mainState,
                                    LSCState *curState);

/**
 状态监视器
 */
@interface LSCStateWatcher : NSObject

/**
 监视事件集合
 */
@property (nonatomic, readonly) LSCStateWatcherEvents events;

/**
 初始化

 @param events 监视事件，可以多个，使用|进行并集
 @return 实例对象
 */
- (instancetype)initWithEvents:(LSCStateWatcherEvents)events;

/**
 当事件触发时调用

 @param handler 事件处理器
 */
- (void)onTrigger:(LSCWatchEventHandler)handler;

/**
 派发事件

 @param context 上下文
 @param mainState 主状态
 @param curState 当前状态
 */
- (void)dispatchEvent:(LSCContext *)context
            mainState:(LSCMainState *)mainState
             curState:(LSCState *)curState;

@end

NS_ASSUME_NONNULL_END
