//
//  LSCWeakReference.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/12.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 弱引用对象
 */
@interface LSCWeakReference : NSObject

/**
 目标引用对象
 */
@property (nonatomic, weak, readonly) id target;

/**
 初始化对象

 @param target 目标引用对象
 @return 对象实例
 */
- (instancetype)initWithTarget:(id)target;

@end

NS_ASSUME_NONNULL_END
