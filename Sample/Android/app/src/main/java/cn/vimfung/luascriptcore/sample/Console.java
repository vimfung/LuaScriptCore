package cn.vimfung.luascriptcore.sample;

import android.util.Log;
import cn.vimfung.luascriptcore.LuaExportType;

/**
 * Created by vimfung on 17/1/23.
 */

public class Console implements LuaExportType
{
    public static void log (String msg)
    {
        Log.v("lsc", msg);
    }
}
