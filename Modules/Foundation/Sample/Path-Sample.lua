require("Foundation")

-- 获取应用路径示例
function Path_Sample_appPath()

	print(Path:appPath());

end

-- 获取沙箱根目录示例
function Path_Sample_homePath()
	
	print(Path:homePath());

end

-- 获取缓存目录示例
function Path_Sample_cachesPath()
	
	print(Path:cachesPath());

end

-- 获取文档目录示例
function Path_Sample_docsPath()
	
	print (Path:docsPath());

end

-- 获取临时目录示例
function Path_Sample_tmpPath()
	
	print (Path:tmpPath());

end