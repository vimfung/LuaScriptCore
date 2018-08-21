//
//  LSCHTTPTask.swift
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#if os(iOS)

import LuaScriptCore_iOS_Swift

#elseif os(OSX)

import LuaScriptCore_OSX_Swift

#endif

/// HTTP任务
@objc(LSCHTTPTask)
class LSCHTTPTask: NSObject, LuaExportType
{
    var _request : URLRequest?;
    var _resultHandler : LuaFunction?;
    var _faultHandler : LuaFunction?;
    var _uploadProgressHandler : LuaFunction?;
    var _downloadProgressHandler : LuaFunction?;
    
    var _session : URLSession?;
    var _task : URLSessionDataTask?;
    
    var _responseData : Data?;
    var _response : HTTPURLResponse?;
    var _contentLength : UInt64 = 0;
    
    var _optQueue : DispatchQueue?;
    var _selfReference : LSCHTTPTask?;
    var _timer : Timer?;
    
    /// 请求地址
    @objc var url : String?
    {
        set
        {
            if newValue == nil
            {
                _request = nil;
                return;
            }
            
            let url : URL? = URL(string: newValue!);
            if url == nil
            {
                _request = nil;
                return;
            }
            
            if _request == nil
            {
                _request = URLRequest(url: url!);
            }
            else
            {
                _request?.url = url;
            }
        }
        get
        {
            return _request?.url?.absoluteString;
        }
    }
    
    
    /// 请求头集合
    @objc var headers : Dictionary<String, String>?;
    
    /// 超时时间
    @objc var timeout : TimeInterval = 60;
    
    
    /// 发起GET请求
    ///
    /// - Parameters:
    ///   - reusltHandler: 返回回调
    ///   - faultHandler: 失败回调
    @objc func get(resultHandler : LuaFunction?, faultHandler : LuaFunction?) -> Void
    {
        _resultHandler = resultHandler;
        _faultHandler = faultHandler;

        _request?.httpMethod = "GET";
        
        _sendRequest();
    }
    
    
    /// 发起POST请求
    ///
    /// - Parameters:
    ///   - parameters: 请求参数
    ///   - resultHandler: 返回回调
    ///   - faultHandler: 失败回调
    @objc func post(parameters : Dictionary<String, String>?,
                    resultHandler : LuaFunction?,
                    faultHandler : LuaFunction?) -> Void
    {
        _resultHandler = resultHandler;
        _faultHandler = faultHandler;
        
        _request?.httpMethod = "POST";
        
        if parameters != nil
        {
            let paramStr = _paramsString(params: parameters!);
            _request?.httpBody = paramStr.data(using: .utf8);
        }
        
        _sendRequest();
    }
    
    
    /// 上传文件
    ///
    /// - Parameters:
    ///   - fileParams: 文件参数集合
    ///   - parameters: 请求参数集合
    ///   - resultHandler: 返回回调
    ///   - faultHandler: 失败回调
    ///   - progressHandler: 上传进度回调
    @objc func upload(fileParams : Dictionary<String, LSCHTTPFile>,
                      parameters : Dictionary<String, String>?,
                      resultHandler : LuaFunction?,
                      faultHandler : LuaFunction?,
                      progressHandler : LuaFunction?) -> Void
    {
        _resultHandler = resultHandler;
        _faultHandler = faultHandler;
        _uploadProgressHandler = progressHandler;
        
        _request?.httpMethod = "POST";
        
        let tmpBoundary : String = "\(arc4random() % (9999999 - 123400) + 123400)";
        let boundaryString : String = "Boundary-\(tmpBoundary)";
        
        _request?.addValue("multipart/form-data; boundary=\(boundaryString)",
            forHTTPHeaderField: "Content-Type");
        
        _request?.httpBody = _multipartForm(fileParams: fileParams,
                                            parameters: parameters,
                                            boundaryString: boundaryString);
        
        _sendRequest();
    }
    
    
    /// 下载文件
    ///
    /// - Parameters:
    ///   - resultHandler: 返回回调
    ///   - faultHandler: 失败回调
    ///   - progressHandler: 下载进度回调
    @objc func download(resultHandler : LuaFunction?,
                        faultHandler : LuaFunction?,
                        progressHandler : LuaFunction?) -> Void
    {
        _resultHandler = resultHandler;
        _faultHandler = faultHandler;
        _downloadProgressHandler = progressHandler;
        
        _request?.httpMethod = "GET";
        
        _sendRequest();
    }
    
    
    /// 取消请求
    @objc func cancel() -> Void
    {
        _addQueueOperation {
            [weak self] in
            
            self?._task?.cancel();
            self?._task = nil;
            
            self?._session?.invalidateAndCancel();
            self?._session = nil;
            
            self?._selfReference = nil;
        };
    }
    
    
    /// 添加队列操作
    ///
    /// - Parameter handler: 处理器
    func _addQueueOperation(handler: @escaping () -> Void) -> Void
    {
        if _optQueue == nil
        {
            _optQueue = DispatchQueue(label:"HTTPOptQueue");
        }

        _optQueue?.async(execute: handler);
    }
    
    /// 发送请求
    func _sendRequest() -> Void
    {
        cancel();
        
        _selfReference = self;
        
        _addQueueOperation {
            [weak self] in
            
            self?._responseData = Data();
        
            DispatchQueue.main.async {
                
                self?._addTimeoutListener();
                
                let config : URLSessionConfiguration = URLSessionConfiguration.default;
                config.timeoutIntervalForRequest = (self?.timeout)!;
                
                self?._session = URLSession(configuration: config,
                                            delegate: self,
                                            delegateQueue: OperationQueue.main);
                self?._task = self?._session?.dataTask(with: (self?._request)!);
                self?._task?.resume();
            };
        }
    }
    
    
    /// 添加超时监听
    func _addTimeoutListener () -> Void
    {
        _removeTimeoutListener();
        _timer = Timer(timeInterval: timeout,
                       target: self,
                       selector: #selector(_timeoutHandler(timer:)),
                       userInfo: nil,
                       repeats: false);
    }
    
    
    /// 超时处理器
    ///
    /// - Parameter timer: 定时器
    @objc private func _timeoutHandler (timer : Timer) -> Void
    {
        cancel();
        _removeTimeoutListener();
    }
    
    
    /// 移除超时监听
    func _removeTimeoutListener () -> Void
    {
        _timer?.invalidate();
        _timer = nil;
    }
    
    
    /// 转换参数为参数字符串
    ///
    /// - Parameter params: 参数集合
    /// - Returns: 参数字符串
    func _paramsString(params : Dictionary<String, String>) -> String
    {
        var requestString : String = "";

        for (key, value) in params
        {
            let encodeKey : String = LSCEncoding.urlEncode(text: key)!;
            let encodeValue : String = LSCEncoding.urlEncode(text: value)!;
            requestString.append("\(encodeKey)=\(encodeValue)&");
        }
        
        if requestString.count > 0
        {
            //移除最后一个字符
            requestString.remove(at: requestString.index(before: requestString.endIndex));
        }
        
        return requestString;
    }
    
    /// 组织Multipart表单数据
    ///
    /// - Parameters:
    ///   - fileParams: 文件参数集合
    ///   - parameters: 参数集合
    ///   - boundaryString: 分隔符
    /// - Returns: 表单数据
    func _multipartForm(fileParams : Dictionary<String, LSCHTTPFile>,
                        parameters : Dictionary<String, String>?,
                        boundaryString : String) -> Data?
    {
        var postData : Data = Data();
        postData.append("--\(boundaryString)\r\n".data(using: .utf8)!);
        
        let endItemBoundaryData : Data = "\r\n--\(boundaryString)\r\n".data(using: .utf8)!;
        
        let fileManager : FileManager = FileManager.default;
        
        var index : Int = 0;
        for (key, value) in fileParams
        {
            if fileManager.fileExists(atPath: value.path!)
            {
                do
                {
                    let fileUrl : URL = URL(fileURLWithPath: value.path!);
                    let fileData = try Data(contentsOf: fileUrl);
                    
                    let itemDescData : Data? = "Content-Disposition: form-data; name=\"\(key)\"; filename=\"\(fileUrl.lastPathComponent)\"\r\n".data(using: .utf8);
                    postData.append(itemDescData!);
                    
                    let mimeType : String = value.mimeType;
                    let contentTypeData : Data? = "Content-Type: \(mimeType)\r\n".data(using: .utf8);
                    postData.append(contentTypeData!);
                    
                    if value.transferEncoding != nil
                    {
                        let transEncoding : String = value.transferEncoding!;
                        let tranfEncodingData : Data? = "Content-Transfer-Encoding: \(transEncoding)\r\n".data(using: .utf8);
                        postData.append(tranfEncodingData!);
                    }
                    
                    postData.append("\r\n".data(using: .utf8)!);
                    
                    postData.append(fileData);
                    
                    if index < fileParams.count - 1
                    {
                        postData.append(endItemBoundaryData);
                    }
                }
                catch
                {
                    
                }
            }
            
            index += 1;
        }
        
        if parameters != nil
        {
            index = 0;
            for (key, value) in parameters!
            {
                let itemDescData : Data? = "Content-Disposition: form-data; name=\"%\(key)\"\r\n\r\n".data(using: .utf8);
                postData.append(itemDescData!);
                
                let itemValueData : Data? = value.data(using: .utf8);
                postData.append(itemValueData!);
                
                if index < fileParams.count - 1
                {
                    postData.append(endItemBoundaryData);
                }
                
                index += 1;
            }
        }
        
        let endData : Data? = "\r\n--\(boundaryString)--\r\n".data(using: .utf8);
        postData.append(endData!);
        
        return postData;
    }
}

extension LSCHTTPTask : URLSessionDelegate
{
    
}

extension LSCHTTPTask : URLSessionTaskDelegate
{
    func urlSession(_ session: URLSession, didReceive challenge: URLAuthenticationChallenge, completionHandler: @escaping (URLSession.AuthChallengeDisposition, URLCredential?) -> Void)
    {
        var credential : URLCredential? = nil;
        var disposition : URLSession.AuthChallengeDisposition = .rejectProtectionSpace;
        
        if challenge.protectionSpace.authenticationMethod == NSURLAuthenticationMethodServerTrust
        {
            credential = URLCredential(trust: challenge.protectionSpace.serverTrust!);
            disposition = .useCredential;
        }
        
        completionHandler(disposition, credential);
    }
    
    func urlSession(_ session: URLSession, task: URLSessionTask, didSendBodyData bytesSent: Int64, totalBytesSent: Int64, totalBytesExpectedToSend: Int64) {
        
        if _uploadProgressHandler != nil
        {
            let args : Array<LuaValue> = [LuaValue(intValue: totalBytesExpectedToSend), LuaValue(intValue: totalBytesSent)];
            _ = _uploadProgressHandler?.invoke(arguments: args);
        }

    }
    
}

extension LSCHTTPTask : URLSessionDataDelegate
{
    func urlSession(_ session: URLSession, dataTask: URLSessionDataTask, didReceive data: Data)
    {
        _responseData?.append(data);
        
        if _downloadProgressHandler != nil
        {
            let args : Array<LuaValue> = [LuaValue(intValue: _contentLength), LuaValue(intValue: (_responseData?.count)!)];
            _ = _downloadProgressHandler?.invoke(arguments: args);
        }
    }
    
    func urlSession(_ session: URLSession, dataTask: URLSessionDataTask, didReceive response: URLResponse, completionHandler: @escaping (URLSession.ResponseDisposition) -> Void)
    {
        _response = response as? HTTPURLResponse;
        
        let contentLength : String? = _response?.allHeaderFields["Content-Length"] as? String;
        _contentLength = UInt64(contentLength!)!;

        completionHandler(.allow);
    }
    
    func urlSession(_ session: URLSession, task: URLSessionTask, didCompleteWithError error: Error?)
    {
        _removeTimeoutListener();
        
        if error != nil
        {
            if _faultHandler != nil
            {
                _ = _faultHandler?.invoke(arguments: [LuaValue(stringValue: (error?.localizedDescription)!)]);
            }
        }
        else
        {
            if _resultHandler != nil
            {
                let args : Array<LuaValue> = [LuaValue(intValue: (_response?.statusCode)!), LuaValue(dataValue: _responseData!)];
                _ = _resultHandler?.invoke(arguments: args);
            }
        }
        
        _session?.finishTasksAndInvalidate();
        
        _task = nil;
        _session = nil;
        _selfReference = nil;
    }
}
