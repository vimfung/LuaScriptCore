//
//  LSCClassInstance.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCClassInstance.h"

@interface LSCClassInstance ()

/**
 *  所属类型
 */
@property (nonatomic, strong) LSCClass *ownerClass;

@end

@implementation LSCClassInstance

- (instancetype)initWithClass:(LSCClass *)ownerClass
{
    if (self = [super init])
    {
        self.ownerClass = ownerClass;
    }
    
    return self;
}

@end
