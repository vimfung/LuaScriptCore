//
//  LSCConfig.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/4/4.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN


/**
 配置信息
 */
@interface LSCConfig : NSObject

/**
 是否使用完整名称导出
 */
@property (nonatomic) BOOL fullExportName;

/**
 获取默认配置

 @return 配置信息
 */
+ (instancetype)defaultConfig;

@end

NS_ASSUME_NONNULL_END
