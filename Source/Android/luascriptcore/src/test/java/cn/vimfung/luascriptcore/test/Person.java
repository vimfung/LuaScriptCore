package cn.vimfung.luascriptcore.test;

import cn.vimfung.luascriptcore.LuaExportType;

/**
 * Created by vimfung on 2018/4/27.
 */

public class Person implements LuaExportType
{
    public Person ()
    {
        Env.sharedInstance().getContext().raiseException("can't create context");
    }
}
