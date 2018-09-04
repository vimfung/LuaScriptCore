//
//  HTTPTask.m
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/20.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCHTTPTask.h"
#import "LSCEncoding.h"
#import "LSCHTTPFile.h"

@interface LSCHTTPTask () <NSURLSessionDelegate,
                           NSURLSessionTaskDelegate,
                           NSURLSessionDataDelegate,
                           LSCExportTypeAnnotation>
{
@private
    
    //处理队列
    dispatch_queue_t _queue;
    //超时计时器
    NSTimer *_timer;
    //自身引用
    LSCHTTPTask *_selfReference;
    
    //请求对象
    NSMutableURLRequest *_request;
    //请求会话
    NSURLSession *_session;
    //请求会话任务
    NSURLSessionDataTask *_task;
    
    //回复数据
    NSMutableData *_responseData;
    //回复对象
    NSHTTPURLResponse *_response;
    //数据长度
    long long _contentLength;
    
    //回复回调
    LSCFunction *_resultHandler;
    //失败回调
    LSCFunction *_faultHandler;
    //上传回调
    LSCFunction *_uploadProgressHandler;
    //下载回调
    LSCFunction *_downloadProgressHandler;
}

@end

@implementation LSCHTTPTask

- (instancetype)init
{
    if (self = [super init])
    {
        self.timeout = 60;
    }
    return self;
}

- (void)setUrl:(NSString *)url
{
    if (!url)
    {
        _request = nil;
        return;
    }
    
    if (!_request)
    {
        _request = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:url]];
    }
    else
    {
        _request.URL = [NSURL URLWithString:url];
    }
}

- (NSString *)url
{
    return _request.URL.absoluteString;
}


- (void)getWithResultHandler:(LSCFunction *)resultHandler
                faultHandler:(LSCFunction *)faultHandler
{
    _resultHandler = resultHandler;
    _faultHandler = faultHandler;
    
    _request.HTTPMethod = @"GET";
    [self _fillHttpHeaders];
    [self _sendRequest];
}

- (void)postWithParameters:(NSDictionary<NSString *, NSString *> *)parameters
             resultHandler:(LSCFunction *)resultHandler
              faultHandler:(LSCFunction *)faultHandler
{
    _resultHandler = resultHandler;
    _faultHandler = faultHandler;
    
    _request.HTTPMethod = @"POST";
    [self _fillHttpHeaders];
    
    if (parameters)
    {
        NSString *paramString = [self _parametersStringWithParameters:parameters];
        _request.HTTPBody = [paramString dataUsingEncoding:NSUTF8StringEncoding];
    }
    
    [self _sendRequest];
}

- (void)uploadWithFileParams:(NSDictionary<NSString *, LSCHTTPFile *> *)fileParams
                  parameters:(NSDictionary<NSString *, NSString *> *)parameters
               resultHandler:(LSCFunction *)resultHandler
                faultHandler:(LSCFunction *)faultHandler
             progressHandler:(LSCFunction *)progressHandler
{
    _resultHandler = resultHandler;
    _faultHandler = faultHandler;
    _uploadProgressHandler = progressHandler;
    
    _request.HTTPMethod = @"POST";
    [self _fillHttpHeaders];
    
    NSString *tmpBoundary = [NSString stringWithFormat:@"%u", arc4random() % (9999999 - 123400) + 123400];
    NSString *boundaryString = [NSString stringWithFormat:@"Boundary-%@", tmpBoundary];
    [_request addValue:[NSString stringWithFormat:@"multipart/form-data; boundary=%@", boundaryString]
    forHTTPHeaderField:@"Content-Type"];
    
    _request.HTTPBody = [self _multipartFormDataWithFileParams:fileParams
                                                    parameters:parameters
                                                boundaryString:boundaryString];
    
    [self _sendRequest];
}

- (void)downloadWithResultHandler:(LSCFunction *)resultHandler
                     faultHandler:(LSCFunction *)faultHandler
                  progressHandler:(LSCFunction *)progressHandler
{
    _resultHandler = resultHandler;
    _faultHandler = faultHandler;
    _downloadProgressHandler = progressHandler;
    
    _request.HTTPMethod = @"GET";
    [self _fillHttpHeaders];
    [self _sendRequest];
}

- (void)cancel
{
    __block typeof(self) theTask = self;
    [self _addQueueOperation:^{
        
        [theTask -> _task cancel];
        theTask -> _task = nil;
        
        [theTask -> _session invalidateAndCancel];
        theTask -> _session = nil;
        
        theTask -> _selfReference = nil;
        
    }];
}

#pragma mark - Private

/**
 填充HTTP请求头
 */
- (void)_fillHttpHeaders
{
    if (self.headers)
    {
        //设置HTTP头
        __block typeof(self) theTask = self;
        [self.headers enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, NSString * _Nonnull obj, BOOL * _Nonnull stop) {
            
            [theTask -> _request setValue:obj forHTTPHeaderField:key];
            
        }];
    }
}

/**
 添加队列操作

 @param handler 操作处理
 */
- (void)_addQueueOperation:(void (^)(void))handler
{
    if (!_queue)
    {
        _queue = dispatch_queue_create("HTTPOptQueue", DISPATCH_QUEUE_SERIAL);
    }
    
    dispatch_async(_queue, handler);
}

/**
 发起请求
 */
- (void)_sendRequest
{
    [self cancel];
    
    _selfReference = self;
    
    __block typeof(self) theTask = self;
    [self _addQueueOperation:^{
        
        if (theTask -> _responseData == nil)
        {
            theTask -> _responseData = [NSMutableData data];
        }
        [theTask -> _responseData setData:[NSData data]];
        
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            
            [theTask _addTimeoutListener];
            
            NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
            config.timeoutIntervalForRequest = theTask.timeout;
            
            theTask -> _session = [NSURLSession sessionWithConfiguration:config delegate:theTask delegateQueue:[NSOperationQueue mainQueue]];
            
            theTask -> _task = [theTask -> _session dataTaskWithRequest:theTask -> _request];
            [theTask -> _task resume];
            
        });
        
    }];
}

/**
 添加超时监听
 */
- (void)_addTimeoutListener
{
    [self _removeTimeoutListener];
    
    //加入超时检测
    _timer = [NSTimer scheduledTimerWithTimeInterval:self.timeout
                                              target:self
                                            selector:@selector(_timeoutHandler)
                                            userInfo:nil
                                             repeats:NO];
}

- (void)_timeoutHandler
{
    //取消请求
    [self cancel];
    [self _removeTimeoutListener];
}

/**
 移除超时检测
 */
- (void)_removeTimeoutListener
{
    [_timer invalidate];
    _timer = nil;
}

/**
 生成参数字符串

 @param parameters 参数集合
 @param encoding 字符串编码
 @return 参数字符串
 */
- (NSString *)_parametersStringWithParameters:(NSDictionary<NSString *, NSString *> *)parameters
{
    NSMutableString *requestString = [NSMutableString string];

    [parameters enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, NSString * _Nonnull obj, BOOL * _Nonnull stop) {
       
        [requestString appendFormat:@"%@=%@&",
         [LSCEncoding urlEncode:key],
         [LSCEncoding urlEncode:obj]];
        
    }];
    
    if (requestString.length > 0)
    {
        [requestString deleteCharactersInRange:NSMakeRange(requestString.length - 1, 1)];
    }
    
    return requestString;
}


/**
 将提交参数转换为MultipartForm组织形式数据

 @param fileParams 文件参数
 @param parameters 参数
 @param boundaryString 分隔字符串
 @param encoding 编码
 @return 转换后数据
 */
- (NSData *)_multipartFormDataWithFileParams:(NSDictionary<NSString *, LSCHTTPFile *> *)fileParams
                                  parameters:(NSDictionary<NSString *, NSString *> *)parameters
                                boundaryString:(NSString *)boundaryString
{
    NSMutableData *postData = [NSMutableData data];
    [postData appendData:[[NSString stringWithFormat:@"--%@\r\n", boundaryString] dataUsingEncoding:NSUTF8StringEncoding]];
    
    NSData *endItemBoundaryData = [[NSString stringWithFormat:@"\r\n--%@\r\n", boundaryString] dataUsingEncoding:NSUTF8StringEncoding];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray<NSString *> *keyArray = [fileParams allKeys];
    NSInteger keyCount = [keyArray count] - 1;
    for (int i = 0; i < keyArray.count; i++)
    {
        NSString *fileKey = keyArray[i];
        LSCHTTPFile *file = fileParams[fileKey];
        
        if ([fileManager fileExistsAtPath:file.path])
        {
            [postData appendData:[[NSString stringWithFormat:
                                   @"Content-Disposition: form-data; name=\"%@\"; filename=\"%@\"\r\n",
                                   fileKey,
                                   [file.path lastPathComponent]]
                                  dataUsingEncoding:NSUTF8StringEncoding]];
            
            [postData appendData:[[NSString stringWithFormat:
                                   @"Content-Type: %@\r\n",
                                   file.mimeType]
                                  dataUsingEncoding:NSUTF8StringEncoding]];
            
            if (file.transferEncoding)
            {
                [postData appendData:[[NSString stringWithFormat:
                                       @"Content-Transfer-Encoding: %@\r\n",
                                       file.transferEncoding]
                                      dataUsingEncoding:NSUTF8StringEncoding]];
            }
            
            [postData appendData:[[NSString stringWithFormat:@"\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
            
            [postData appendData:[NSData dataWithContentsOfFile:file.path]];
            
            if (i < keyCount)
            {
                [postData appendData:endItemBoundaryData];
            }
        }
        
    }

    keyArray = [parameters allKeys];
    keyCount = [keyArray count] - 1;
    for (int i = 0; i < [keyArray count]; i++)
    {
        NSString *keyString = [keyArray objectAtIndex:i];
        NSString *value = parameters[keyString];
        
        [postData appendData:[[NSString stringWithFormat:
                               @"Content-Disposition: form-data; name=\"%@\"\r\n\r\n",
                               keyString]
                              dataUsingEncoding:NSUTF8StringEncoding]];
        [postData appendData:[value dataUsingEncoding:NSUTF8StringEncoding]];

        if (i < keyCount)
        {
            [postData appendData:endItemBoundaryData];
        }
    }

    [postData appendData:[[NSString stringWithFormat:
                           @"\r\n--%@--\r\n",
                           boundaryString]
                          dataUsingEncoding:NSUTF8StringEncoding]];

    return postData;
}

#pragma mark - LSCExportTypeAnnotation

+ (NSArray<NSString *> *)excludeExportInstanceMethods
{
    return @[NSStringFromSelector(@selector(URLSession:didReceiveChallenge:completionHandler:)),
             NSStringFromSelector(@selector(URLSession:task:didSendBodyData:totalBytesSent:totalBytesExpectedToSend:)),
             NSStringFromSelector(@selector(URLSession:dataTask:didReceiveData:)),
             NSStringFromSelector(@selector(URLSession:dataTask:didReceiveResponse:completionHandler:)),
             NSStringFromSelector(@selector(URLSession:task:didCompleteWithError:))];
}

#pragma mark - NSURLSessionTaskDelegate

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
 completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * _Nullable credential))completionHandler
{
    NSURLCredential *credential = nil;
    NSURLSessionAuthChallengeDisposition disposition = NSURLSessionAuthChallengeRejectProtectionSpace;
    if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust])
    {
        //如果是受信任证书则使用证书
        credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        disposition = NSURLSessionAuthChallengeUseCredential;
    }
    
    if (completionHandler)
    {
        completionHandler(disposition, credential);
    }
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
   didSendBodyData:(int64_t)bytesSent
    totalBytesSent:(int64_t)totalBytesSent
totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend
{
    if (_uploadProgressHandler)
    {
        NSArray<LSCValue *> *args = @[[LSCValue numberValue:@(totalBytesExpectedToSend)],
                                      [LSCValue numberValue:@(totalBytesSent)]];
        [_uploadProgressHandler invokeWithArguments:args];
    }
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
    didReceiveData:(NSData *)data
{
    [_responseData appendData:data];
    
    if (_downloadProgressHandler)
    {
        NSArray<LSCValue *> *args = @[[LSCValue numberValue:@(_contentLength)],
                                      [LSCValue numberValue:@(_responseData.length)]];
        [_downloadProgressHandler invokeWithArguments:args];
    }
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)response
 completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler
{
    _response = (NSHTTPURLResponse *)response;
    _contentLength = [[_response allHeaderFields][@"Content-Length"] longLongValue];
    
    if (completionHandler)
    {
        completionHandler(NSURLSessionResponseAllow);
    }
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
didCompleteWithError:(nullable NSError *)error
{
    //取消超时计时器
    [self _removeTimeoutListener];
    
    if (error)
    {
        //请求错误
        if (_faultHandler)
        {
            [_faultHandler invokeWithArguments:@[[LSCValue stringValue:error.localizedFailureReason]]];
        }
        
        [_session finishTasksAndInvalidate];
        
        _task = nil;
        _session = nil;
        _selfReference = nil;
    }
    else
    {
        //请求成功
        
        if (_resultHandler)
        {
            NSArray<LSCValue *> *args = @[[LSCValue integerValue:_response.statusCode],
                                          [LSCValue dataValue:_responseData]];
            [_resultHandler invokeWithArguments:args];
        }
        
        [_session finishTasksAndInvalidate];
        _task = nil;
        _session = nil;
        _selfReference = nil;
    }
}

@end
