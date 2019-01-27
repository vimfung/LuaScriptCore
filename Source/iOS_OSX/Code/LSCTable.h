//
//  LSCTable.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/1/16.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LSCContext;

@interface LSCTable : NSObject

/**
 初始化

 @param dictionary 字典
 @param objectId 对象标识
 @param context 上下文对象
 @return table对象
 */
- (instancetype)initWithDictionary:(NSDictionary *)dictionary
                          objectId:(NSString * _Nullable)objectId
                           context:(LSCContext * _Nullable)context;

/**
 初始化

 @param array 数组
 @param objectId 对象标识
 @param context 上下文对象
 @return table对象
 */
- (instancetype)initWithArray:(NSArray *)array
                     objectId:(NSString * _Nullable)objectId
                      context:(LSCContext * _Nullable)context;

/**
 设置指定键值对

 @param object 对象
 @param keyPath 键名路径
 */
- (void)setObject:(id)object forKeyPath:(NSString *)keyPath;

@end

NS_ASSUME_NONNULL_END
