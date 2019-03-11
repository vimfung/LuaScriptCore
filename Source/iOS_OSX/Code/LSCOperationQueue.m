//
//  LSCOperationQueue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/6/28.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCOperationQueue.h"
#import "LSCEngineAdapter.h"
#import "LSCSession_Private.h"
#import <pthread.h>

#import "ldo.h"

@interface LSCOperationQueue ()
{
@private
    pthread_mutex_t _lock;
}

@end

@implementation LSCOperationQueue

- (instancetype)init
{
    if (self = [super init])
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        
        pthread_mutex_init(&_lock, &attr);
        
        pthread_mutexattr_destroy(&attr);
    }
    
    return self;
}

- (void)dealloc
{
    pthread_mutex_destroy(&_lock);
}

- (void)performAction:(void (^)(void))block
{
    pthread_mutex_lock(&_lock);
    block();
    pthread_mutex_unlock(&_lock);
}

@end
