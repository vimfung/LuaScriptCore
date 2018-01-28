package cn.vimfung.luascriptcore.sample;

import android.content.Context;

import cn.vimfung.luascriptcore.LuaContext;

/**
 * Created by vimfung on 2017/5/16.
 */
public class Env
{
    private static LuaContext _ctx = null;

    public static void setup(Context context)
    {
        LuaContext.excludeClassesRules.add("^com[.]blankj[.]utilcode[.].+");
        _ctx = LuaContext.create(context);
    }

    public static LuaContext defaultContext ()
    {
        return _ctx;
    }


}
