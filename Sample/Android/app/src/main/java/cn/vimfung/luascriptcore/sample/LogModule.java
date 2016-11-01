package cn.vimfung.luascriptcore.sample;

import android.util.Log;

import cn.vimfung.luascriptcore.LuaModule;

/**
 * 日志模块
 * Created by vimfung on 16/9/23.
 */
public class LogModule extends LuaModule
{
    public static String version()
    {
        return "1.0.0";
    }

    public static void writeLog(String message)
    {
        Log.v("luascriptcore", message);
    }
}
