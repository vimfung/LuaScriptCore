//
//  LSCManagedValue.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/23.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCContext;
@class LSCValue;

/**
 管理值对象，用于维护LSCValue的值所对应的Lua对象在LSCManagedValue存在期间不被释放。
 */
@interface LSCManagedValue : NSObject

/**
 源值对象
 */
@property (nonatomic, strong, readonly) LSCValue *source;

/**
 初始化

 @param value 值
 @param context 上下文对象
 @return 管理值对象
 */
- (instancetype)initWithValue:(LSCValue *)value
                      context:(LSCContext *)context;

@end
