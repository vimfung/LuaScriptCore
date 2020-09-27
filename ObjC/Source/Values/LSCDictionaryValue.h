//
//  LSCDictionaryValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

/**
 字典类型
 */
@interface LSCDictionaryValue : NSObject <LSCValueType>

/**
 初始化对象

 @param dictionary 字典对象
 @return 对象实例
 */
- (instancetype)initWithDictionary:(NSDictionary *)dictionary;

@end

NS_ASSUME_NONNULL_END
