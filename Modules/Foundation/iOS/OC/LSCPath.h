//
//  LSCPath.h
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"

/**
 路径信息
 */
@interface LSCPath : NSObject <LSCExportType>

/**
 应用程序所在目录

 @return 路径信息
 */
+ (NSString *)appPath;

/**
 应用的沙箱根目录

 @return 路径信息
 */
+ (NSString *)homePath;

/**
 应用的文档目录

 @return 路径信息
 */
+ (NSString *)docsPath;

/**
 应用的缓存目录

 @return 路径信息
 */
+ (NSString *)cachesPath;


/**
 获取临时目录

 @return 路径信息
 */
+ (NSString *)tmpPath;

@end
