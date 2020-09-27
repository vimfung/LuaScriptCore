//
//  LSCWeakReference.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/12.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCWeakReference.h"
#import "LSCException.h"

@interface LSCWeakReference ()

/**
 目标引用对象
 */
@property (nonatomic, weak) id target;

@end

@implementation LSCWeakReference

- (instancetype)initWithTarget:(id)target
{
    if (self = [super init])
    {
        self.target = target;
    }
    return self;
}

#pragma mark - Rewrite

- (instancetype)init
{
    @throw CANNOT_INIT_OBJ_EXCEPTION;
}

@end
