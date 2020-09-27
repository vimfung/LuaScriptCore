//
//  LSCClassValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"
#import "LSCTypeDescription.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Lua中的类型
 */
@interface LSCTypeValue : NSObject <LSCValueType>

/**
 初始化对象

 @param typeDescription 类型描述
 @return 对象实例
 */
- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription;

@end

NS_ASSUME_NONNULL_END
