//
//  LSCHTTPFile.h
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"

/**
 HTTP文件
 */
@interface LSCHTTPFile : NSObject <LSCExportType>

/**
 文件路径
 */
@property (nonatomic, copy) NSString *path;

/**
 内容类型
 */
@property (nonatomic, copy) NSString *mimeType;

/**
 内容传输编码
 */
@property (nonatomic, copy) NSString *transferEncoding;

@end
