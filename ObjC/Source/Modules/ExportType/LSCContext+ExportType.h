//
//  LSCContext+ExportType.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/2.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCContext.h"

NS_ASSUME_NONNULL_BEGIN

@class LSCTypeDescription;

@interface LSCContext (ExportType)

/**
 类型描述表
 */
@property (nonatomic, strong) NSMutableDictionary<NSString *, LSCTypeDescription *> *typeDescriptionMap;


/**
 获取类型描述
 
 @param name 类型名称
 @return 类型描述
 */
- (LSCTypeDescription *)typeDescriptionByName:(NSString *)name;

@end

NS_ASSUME_NONNULL_END
