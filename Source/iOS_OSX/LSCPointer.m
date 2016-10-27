//
//  LSCPointer.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCPointer.h"

@interface LSCPointer ()

@property (nonatomic) const void *ptr;

@end

@implementation LSCPointer

- (instancetype)initWithPtr:(const void *)ptr
{
    if (self = [super init])
    {
        self.ptr = ptr;
    }
    
    return self;
}

- (const void *)value
{
    return self.ptr;
}

@end
