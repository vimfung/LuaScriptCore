//
//  LSCPointer.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCPointer.h"
#import "LSCManagedObjectProtocol.h"
#import "LSCContext_Private.h"
#import "LSCSession_Private.h"
#import "LSCEngineAdapter.h"

@interface LSCPointer () <LSCManagedObjectProtocol>

@property (nonatomic) LSCUserdataRef userdataRef;

/**
 是否需要释放内存，对于传入原始指针的构造方法会为指针包装一层LSCPointerRef结构体，因此，在对象释放时需要进行释放LSCPointerRef结构体。
 */
@property (nonatomic) BOOL needFree;

/**
 连接标识
 */
@property (nonatomic, copy) NSString *_linkId;

@end

@implementation LSCPointer

- (instancetype)initWithUserdata:(LSCUserdataRef)ref
{
    if (self = [super init])
    {
        self.needFree = NO;
        self.userdataRef = ref;
    }
    return self;
}

- (instancetype)initWithPtr:(const void *)ptr
{
    if (self = [super init])
    {
        self.needFree = YES;
        
        self.userdataRef = malloc(sizeof(LSCUserdataRef));
        self.userdataRef -> value = (void *)ptr;
        
    }
    return self;
}

- (void)dealloc
{
    if (self.needFree)
    {
        free(self.userdataRef);
        self.userdataRef = NULL;
    }
}

- (const LSCUserdataRef)value
{
    return self.userdataRef;
}

#pragma mark - LSCManagedObjectProtocol

- (NSString *)linkId
{
    if (!self._linkId)
    {
        self._linkId = [NSString stringWithFormat:@"%p", [self value]];
    }
    
    return self._linkId;
}

- (BOOL)pushWithContext:(LSCContext *)context
{
    [self pushWithState:context.currentSession.state queue:context.optQueue];
    return YES;
}

- (BOOL)pushWithState:(lua_State *)state queue:(LSCOperationQueue *)queue
{
    __weak typeof(self) thePointer = self;
    void (^handler) (void) = ^{
        [LSCEngineAdapter pushLightUserdata:[thePointer value] state:state];
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler ();
    }
    
    return YES;
}

@end
