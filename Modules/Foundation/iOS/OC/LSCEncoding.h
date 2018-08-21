//
//  Encoding.h
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"

/**
 编码工具类型
 */
@interface LSCEncoding : NSObject <LSCExportType>

/**
 *  URL编码
 *
 *  @param string   原始字符串
 *
 *  @return 编码后字符串
 */
+ (NSString *)urlEncode:(NSString *)string;

/**
 *  URL解码
 *
 *  @param string   原始字符串
 *
 *  @return 解码后字符串
 */
+ (NSString *)urlDecode:(NSString *)string;

@end
