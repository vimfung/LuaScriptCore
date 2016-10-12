package cn.vimfung.luascriptcore.sample;

import android.util.Log;

import cn.vimfung.luascriptcore.modules.oo.LuaObjectClass;

/**
 * 人类
 * Created by vimfung on 16/10/12.
 */
public class Person extends LuaObjectClass
{
    public String name;

    public void speak()
    {
        Log.v("luascriptcore", String.format("%s speak", name));
    }

    public void walk()
    {
        Log.v("luascriptcore", String.format("%s walk", name));
    }
}
