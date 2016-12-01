//
//  LSCPointer.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCPointer.h"

@interface LSCPointer ()

@property (nonatomic) LSCUserdataRef userdataRef;

/**
 是否需要释放内存，对于传入原始指针的构造方法会为指针包装一层LSCPointerRef结构体，因此，在对象释放时需要进行释放LSCPointerRef结构体。
 */
@property (nonatomic) BOOL needFree;

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

@end
