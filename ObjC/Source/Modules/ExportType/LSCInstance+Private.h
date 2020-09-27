//
//  LSCInstance+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCInstance.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCTypeDescription;

@interface LSCInstance ()

/**
 实例标识，用于记录对应lua中的对象
 */
@property (nonatomic, readonly, copy) NSString *instanceId;

/**
 所属上下文
 */
@property (nonatomic, weak, readonly) LSCContext *ownerContext;


/**
 初始化

 @param instanceId 实例Id
 @param typeDescription 类型
 @param object 对象
 @param context 上下文
 @return 实例对象
 */
- (instancetype)_initWithInstanceId:(NSString *)instanceId
                    typeDescription:(LSCTypeDescription *)typeDescription
                             object:(nullable id)object
                            context:(LSCContext *)context;

@end

NS_ASSUME_NONNULL_END
