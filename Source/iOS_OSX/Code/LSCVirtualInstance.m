//
//  LSCObjectDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/12/27.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCVirtualInstance.h"

@implementation LSCVirtualInstance

- (instancetype)initWithTypeDescriptor:(LSCExportTypeDescriptor *)typeDescriptor
{
    if (self = [super init])
    {
        _typeDescriptor = typeDescriptor;
    }
    
    return self;
}

@end
