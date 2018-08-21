//
//  HTTPTask.h
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"

@class LSCHTTPFile;

/**
 HTTP任务
 */
@interface LSCHTTPTask : NSObject <LSCExportType>

/**
 链接
 */
@property (nonatomic, copy) NSString *url;

/**
 表头参数
 */
@property (nonatomic, strong) NSDictionary<NSString *, NSString *> *headers;

/**
 超时时间，默认1分钟
 */
@property (nonatomic) NSTimeInterval timeout;


/**
 发起GET请求

 @param resultHandler 返回回调
 @param faultHandler 失败回调
 */
- (void)getWithResultHandler:(LSCFunction *)resultHandler
                faultHandler:(LSCFunction *)faultHandler;

/**
 发起POST请求

 @param parameters 请求参数
 @param resultHandler 返回回调
 @param faultHandler 失败回调
 */
- (void)postWithParameters:(NSDictionary<NSString *, NSString *> *)parameters
             resultHandler:(LSCFunction *)resultHandler
              faultHandler:(LSCFunction *)faultHandler;

/**
 上传文件

 @param fileParams 文件参数，Key为文件参数名称，Value为HTTP文件对象
 @param parameters 参数请求
 @param resultHandler 返回回调
 @param faultHandler 失败回调
 @param progressHandler 进度回调
 */
- (void)uploadWithFileParams:(NSDictionary<NSString *, LSCHTTPFile *> *)fileParams
                  parameters:(NSDictionary<NSString *, NSString *> *)parameters
               resultHandler:(LSCFunction *)resultHandler
                faultHandler:(LSCFunction *)faultHandler
            progressHandler:(LSCFunction *)progressHandler;


/**
 下载文件

 @param resultHandler 返回回调
 @param faultHandler 失败回调
 @param progressHandler 进度回调
 */
- (void)downloadWithResultHandler:(LSCFunction *)resultHandler
                     faultHandler:(LSCFunction *)faultHandler
                  progressHandler:(LSCFunction *)progressHandler;

/**
 取消请求
 */
- (void)cancel;

@end
