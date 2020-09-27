//
//  LSCInstance.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/8.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCTypeDescription;

/**
 实例对象，当导出类型创建对象时会统一生成该对象实例
 */
@interface LSCInstance : NSObject

/**
 类型描述
 */
@property (nonatomic, weak, readonly) LSCTypeDescription *typeDescription;

/**
 原始对象
 */
@property (nonatomic, strong, readonly) id object;

/**
 获取属性

 @param key 属性名称
 @param context 上下文
 @return 属性值
 */
- (id<LSCValueType>)getPropertyForKey:(NSString *)key
                              context:(LSCContext *)context;

/**
 设置属性

 @param value 属性值
 @param key 属性名称
 */
- (void)setProperty:(id<LSCValueType>)value
             forKey:(NSString *)key
            context:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
