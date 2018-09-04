package cn.vimfung.luascriptcore.modules.network;

import cn.vimfung.luascriptcore.LuaExportType;

/**
 * HTTP上传文件
 */
public class HTTPFile implements LuaExportType
{
    /**
     * 路径
     */
    public String path;

    /**
     * 内容类型，默认application/octet-stream
     */
    public String mimeType;

    /**
     * 传输编码
     */
    public String transferEncoding;
}
