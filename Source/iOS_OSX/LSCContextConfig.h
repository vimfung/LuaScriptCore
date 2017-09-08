//
//  LSCContextConfig.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/7.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 上下文配置信息
 */
@interface LSCContextConfig : NSObject

/**
 手动导入类型开关，设置为YES时，在lua中需要使用某个原生类型时需要调用nativeType('typeName')方法来导入指定类型。默认为NO。
 设置该开关目的是为了提升执行的效率，因为一般情况下并非是所有导出类都需要一开始就进行加载，如果导出类型很多的情况就会导致启动变慢。
 使用该开关可以合理分配加载类型的时机。
 */
@property (nonatomic) BOOL manualImportClassEnabled;

/**
 获取默认配置信息

 @return 默认配置信息
 */
+ (instancetype)defaultConfig;

@end
