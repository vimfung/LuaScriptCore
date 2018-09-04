package cn.vimfung.luascriptcore.modules.foundation;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;

import cn.vimfung.luascriptcore.LuaExportType;

/**
 * 编码工具类
 */
public final class Encoding implements LuaExportType
{
    /**
     * 对文本进行URL编码
     * @param text  文本内容
     * @return  编码后的文本内容
     */
    public static String urlEncode(String text)
    {
        try
        {
            return URLEncoder.encode(text, "utf-8");
        }
        catch (UnsupportedEncodingException e)
        {
            return "";
        }
    }

    /**
     * 对文本进行URL解码
     * @param text  文本内容
     * @return  解码后文本内容
     */
    public static String urlDecode(String text)
    {
        try
        {
            return URLDecoder.decode(text, "utf-8");
        }
        catch (UnsupportedEncodingException e)
        {
            return "";
        }
    }
}
