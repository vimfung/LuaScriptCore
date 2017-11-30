//
//  LSCExportMethodDescriptor.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/8.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 导出方法描述
 */
@interface LSCExportMethodDescriptor : NSObject

/**
 方法签名，如：@@、@I@B
 */
@property (nonatomic, copy) NSString *methodSignature;

/**
 调用器
 */
@property (nonatomic, strong) NSInvocation *invocation;

@end
