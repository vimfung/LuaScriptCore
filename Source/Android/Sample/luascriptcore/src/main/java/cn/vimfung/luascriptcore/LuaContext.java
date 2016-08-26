package cn.vimfung.luascriptcore;

import java.util.Date;

/**
 * Lua上下文对象
 * Created by vimfung on 16/8/22.
 */
public class LuaContext
{
    static
    {
        System.loadLibrary("LuaScriptCore");
    }

    private String _name;

    private native void createContext (String name);
    private native void releaseContext (String name);
    private native LuaValue evalScript (String contextName, String script);

    public LuaContext()
    {
        _name = "LSC_" + new Date().toString();
        createContext(_name);
    }

    @Override
    protected void finalize() throws Throwable
    {
        releaseContext(_name);
        super.finalize();
    }

    /**
     * 解析Lua脚本
     * @param script  脚本
     * @return 执行后返回的值
     */
    public LuaValue evalScript (String script)
    {

        return evalScript(_name, script);
    }
}
