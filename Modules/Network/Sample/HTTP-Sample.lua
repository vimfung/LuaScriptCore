require("HTTP")

function HTTP_Sample_get()
	
	HTTP:get(
		"https://itunes.apple.com/lookup?id=414478124", 
		function (status, data)
		
			print("http status = ", status);
			print("http response data = ", data);

		end, 
		function (errmsg)
		
			print("http fault = ", errmsg);

		end);

end

function HTTP_Sample_post()
	
	HTTP:post(
		"https://itunes.apple.com/search", 
		{term="微信", entity="software"}, 
		function (status, data)
		
			print("http status = ", status);
			print("http response data = ", data);

		end, 
		function (errmsg)
		
			print("http fault = ", errmsg);

		end);

end

function HTTP_Sample_upload()

	HTTP:upload(
		"https://qiniu-storage.pgyer.com/apiv1/app/upload", 
		Path:appPath() .. "/timg.jpeg", 
		"file", 
		function (status, data)
		
			print("http status = ", status);
			print("http response data = ", data);

		end, 
		function (errmsg)
		
			print("http fault = ", errmsg);

		end,
		function (totalBytes, uploadedBytes)
			
			print("upload progress = ", uploadedBytes, "/", totalBytes);

		end);

end

function HTTP_Sample_download()
	
	HTTP:download(
		"https://timgsa.baidu.com/timg?image&quality=80&size=b9999_10000&sec=1534769061065&di=32c8055b26b5054090158633e60c3e11&imgtype=0&src=http%3A%2F%2Fa.hiphotos.baidu.com%2Fimage%2Fpic%2Fitem%2Fadaf2edda3cc7cd90df1ede83401213fb80e9127.jpg",
		function (status, data)
		
			print("http status = ", status);
			print("http response data = ", data);

		end, 
		function (errmsg)
		
			print("http fault = ", errmsg);

		end,
		function (totalBytes, downloadedBytes)
			
			print("download progress = ", downloadedBytes, "/", totalBytes);

		end);

end
