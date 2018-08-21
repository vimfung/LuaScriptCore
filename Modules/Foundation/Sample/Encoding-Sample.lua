require("Foundation")

-- URL编码示例
function Encoding_Sample_urlEncode()
	
	print(Encoding:urlEncode("https://www.baidu.com/s?wd=你的名字"));

end

-- URL解码示例
function Encoding_Sample_urlDecode()
	
	print(Encoding:urlDecode("https://www.baidu.com/s?wd=%E4%B8%8A%E4%BC%A0"));

end