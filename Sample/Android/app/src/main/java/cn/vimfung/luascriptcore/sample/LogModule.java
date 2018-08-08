package cn.vimfung.luascriptcore.sample;

import android.util.Log;

import cn.vimfung.luascriptcore.LuaExclude;
import cn.vimfung.luascriptcore.LuaExportType;
import cn.vimfung.luascriptcore.LuaTuple;

/**
 * 日志模块
 * Created by vimfung on 16/9/23.
 */
public class LogModule implements LuaExportType
{
    public static void writeLog(String message)
    {
        Log.v("luascriptcore", message);
    }

    public static LuaTuple test(String a, String b)
    {
        LuaTuple tuple = new LuaTuple();
        tuple.addReturnValue(a);
        tuple.addReturnValue(b);

        return tuple;
    }

    public static void testObj(Object obj)
    {
        Log.v("lsc", obj.toString());
    }

    public static Object createObj ()
    {
        return new Object();
    }
}
