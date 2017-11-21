package cn.vimfung.luascriptcore.sample;

import android.app.Application;
import android.content.Context;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaContextConfig;

/**
 * Created by vimfung on 2017/5/16.
 */
public class Env
{
    private static LuaContext _ctx = null;

    public static void setup(Context context)
    {
        LuaContextConfig config = new LuaContextConfig();
        config.manualImportClassEnabled = true;

        _ctx = LuaContext.create(context, config);
    }

    public static LuaContext defaultContext ()
    {
        return _ctx;
    }


}
