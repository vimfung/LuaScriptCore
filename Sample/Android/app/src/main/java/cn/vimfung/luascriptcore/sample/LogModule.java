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

    public void writeLog(String message)
    {
        Log.v("luascriptcore", message);
    }

    public double add(double[] a) {return a[0] + a[1];}

    public int age;

    public String name;

    public void printInfo ()
    {
        Log.v("lsc", String.format("%s age = %d", name, age));
    }
}
