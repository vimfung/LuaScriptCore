//
//  LSCExportPropertyDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/11/24.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportPropertyDescriptor.h"

@interface LSCExportPropertyDescriptor ()

/**
 属性名称
 */
@property (nonatomic, copy) NSString *name;

@end

@implementation LSCExportPropertyDescriptor

- (instancetype)initWithName:(NSString *)name
{
    if (self = [super init])
    {
        self.name = name;
    }
    
    return self;
}

@end
