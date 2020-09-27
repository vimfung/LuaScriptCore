//
//  LuaValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

/**
 用于与Lua层交互数据的对象
 */
@interface LSCValue : NSObject <LSCValueType>

/**
 注册类型，用于扩展
 
 @param valueType 类型
 */
+ (void)registerValueType:(Class<LSCValueType>)valueType;

@end

NS_ASSUME_NONNULL_END
