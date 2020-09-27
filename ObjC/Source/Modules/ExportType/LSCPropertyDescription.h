//
//  LSCPropertyDescription.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/22.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCTypeDescription;
@class LSCInstance;
@class LSCContext;
@class LSCFunctionValue;


/**
 属性描述
 */
@interface LSCPropertyDescription : NSObject

/**
 类型
 */
@property (nonatomic, readonly) LSCTypeDescription *typeDescription;


/**
 初始化

 @param typeDescription 类型
 @param getter 获取方法
 @param setter 设置方法
 @return 属性对象实例
 */
- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
                                 getter:(SEL)getter
                                 setter:(SEL)setter;

/**
 初始化

 @param typeDescription 类型
 @param getterFunc 获取方法
 @param setterFunc 设置方法
 @return 属性对象实例
 */
- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
                             getterFunc:(LSCFunctionValue *)getterFunc
                             setterFunc:(LSCFunctionValue *)setterFunc;

/**
 获取属性值

 @param instance 实例对象
 @param context 上下文
 @return 属性值
 */
- (id<LSCValueType>)getValueWithInstance:(LSCInstance *)instance
                                 context:(LSCContext *)context;

/**
 设置属性值

 @param value 属性值
 @param instance 实例对象
 @param context 上下文
 */
- (void)setValue:(id<LSCValueType>)value
    withInstance:(LSCInstance *)instance
         context:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
