require("Foundation")

Object:typeMapping("ios", "LSCHTTPTask", "HTTPTask");
Object:typeMapping("ios", "LSCHTTPFile", "HTTPFile");

Object:typeMapping("android", "cn.vimfung.luascriptcore.modules.network.HTTPTask", "HTTPTask");
Object:typeMapping("android", "cn.vimfung.luascriptcore.modules.network.HTTPFile", "HTTPFile");

Object:subclass("HTTP");

-- 以GET方式发起HTTP请求
-- @param url 请求地址
-- @param result 返回回调，回调声明形式：function (status, data);
-- @param fault 失败回调，回调声明形式：function (errMsg);
-- @return 任务对象
function HTTP:get(url, result, fault)

	local task = HTTPTask();
	task.url = url;
	task:get(result, fault);

	return task;

end

-- 以POST方式发起HTTP请求
-- @param url 请求地址
-- @param parameters 请求参数，table类型，Key和Value均为字符串
-- @param result 返回回调，回调声明形式：function (status, data);
-- @param fault 失败回调，回调声明形式：function (errMsg);
-- @return 任务对象
function HTTP:post(url, parameters, result, fault)
	
	local task = HTTPTask();
	task.url = url;
	task:post(parameters, result, fault);

	return task;

end

-- 上传文件
-- @param url 请求地址
-- @param filePath 文件路径
-- @param fileKey 文件参数名称，对应服务器端定义参数名称
-- @param result 返回回调，回调声明形式：function (status, data);
-- @param fault 失败回调，回调声明形式：function (errMsg);
-- @param progress 上传进度回调，回调声明形式：function (totalBytes, sentBytes);
-- @return 任务对象
function HTTP:upload(url, filePath, fileKey, result, fault, progress)
	
	local task = HTTPTask();
	task.url = url;

	local httpFile = HTTPFile();
	httpFile.path = filePath;

	task:upload({fileKey=httpFile}, nil, result, fault, progress);

	return task;

end

-- 下载文件
-- @param url 文件的远程地址
-- @param result 返回回调，回调声明形式：function (status, data);
-- @param fault 失败回调，回调声明形式：function (errMsg);
-- @param progress 下传进度回调，回调声明形式：function (totalBytes, downloadedBytes);
-- @return 任务对象
function HTTP:download(url, result, fault, progress)
	
	local task = HTTPTask();
	task.url = url;
	task:download(result, fault, progress);

	return task;

end
