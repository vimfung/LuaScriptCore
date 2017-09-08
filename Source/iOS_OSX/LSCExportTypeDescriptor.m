//
//  LSCExportTypeDescriptor.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportTypeDescriptor.h"

@implementation LSCExportTypeDescriptor

- (BOOL)subtypeOfType:(LSCExportTypeDescriptor *)typeDescriptor
{
    LSCExportTypeDescriptor *targetTypeDescriptor = self;
    while (targetTypeDescriptor)
    {
        if (targetTypeDescriptor == typeDescriptor)
        {
            return YES;
        }
        
        targetTypeDescriptor = targetTypeDescriptor.parentTypeDescriptor;
    }
    
    return NO;
}

@end
