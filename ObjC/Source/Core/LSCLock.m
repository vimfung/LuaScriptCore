//
//  LSCLock.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/11.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCLock.h"
#import <pthread.h>

@interface LSCLock ()

//锁，用于控制Lua操作同步
@property (nonatomic) pthread_mutex_t luaOptLock;

@end

@implementation LSCLock

- (void)lock
{
    pthread_mutex_lock(&_luaOptLock);
}

- (void)unlock
{
    pthread_mutex_unlock(&_luaOptLock);
}

#pragma mark - Rewrite

- (instancetype)init
{
    if (self = [super init])
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        
        pthread_mutex_init(&_luaOptLock, &attr);
        
        pthread_mutexattr_destroy(&attr);
    }
    return self;
}

- (void)dealloc
{
    pthread_mutex_destroy(&_luaOptLock);
}

@end
