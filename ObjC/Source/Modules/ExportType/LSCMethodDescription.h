//
//  LSCMethodDescription.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCTypeDescription;

/**
 方法描述
 */
@interface LSCMethodDescription : NSObject

/**
 类型
 */
@property (nonatomic, weak, readonly) LSCTypeDescription *typeDescription;

/**
 方法名称
 */
@property (nonatomic, copy, readonly) NSString *name;

/**
 选择子
 */
@property (nonatomic, readonly) SEL selector;

/**
 参数签名，如：@@、@I@B
 */
@property (nonatomic, copy, readonly) NSString *paramsSignature;

/**
 返回值签名
 */
@property (nonatomic, copy, readonly) NSString *resultSignature;


/**
 方法签名
 */
@property (nonatomic, strong, readonly) NSMethodSignature *methodSignature;

/**
 初始化
 
 @param typeDescription 类型
 @param selector 选择子
 @param methodSignature 方法签名
 @param paramsSignature 参数签名
 @param resultSignature 返回值签名
 @return 方法描述
 */
- (instancetype)initWithTypeDescription:(LSCTypeDescription *)typeDescription
                               selector:(SEL)selector
                        methodSignature:(NSMethodSignature *)methodSignature
                        paramsSignature:(NSString *)paramsSignature
                        resultSignature:(NSString *)resultSignature;

/**
 调用方法

 @param target 调用对象，当传入typeDescription类型时为类方法，其他对象则为实例方法
 @param arguments 参数列表
 @param context 上下文对象
 @return 返回值，nil 表示无返回值。
 */
- (id<LSCValueType>)invokeWithTarget:(id)target
                           arguments:(NSArray<id<LSCValueType>> *)arguments
                             context:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
