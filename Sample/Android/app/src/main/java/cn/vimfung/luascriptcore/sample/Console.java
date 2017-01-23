package cn.vimfung.luascriptcore.sample;

import android.util.Log;

import cn.vimfung.luascriptcore.modules.oo.LuaObjectClass;

/**
 * Created by vimfung on 17/1/23.
 */

public class Console extends LuaObjectClass
{
    public static void log (String msg)
    {
        Log.v("lsc", msg);
    }
}
