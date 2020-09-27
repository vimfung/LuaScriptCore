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

/**
 将一个对象放入字典中。注：该方法只有在valueType为LSCValueTypeMap时有效
 
 @param object 放入字典的对象
 @param keyPath 对应的键名路径，例如："key"、"key1.key2"
 */
- (void)setObject:(id<LSCValueType>)object
       forKeyPath:(NSString *)keyPath;

@end

NS_ASSUME_NONNULL_END
