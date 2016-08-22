package cn.vimfung.luascriptcore;

import java.util.Date;

/**
 * Created by vimfung on 16/8/22.
 */
public class LuaContext
{

    static
    {
        System.loadLibrary("LuaScriptCore");
    }

    private native void createContext (String name);

    public LuaContext()
    {
        createContext("LSC_" + new Date().toString());
    }
}
