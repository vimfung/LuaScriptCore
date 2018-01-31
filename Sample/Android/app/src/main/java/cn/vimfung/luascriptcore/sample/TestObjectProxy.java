package cn.vimfung.luascriptcore.sample;

import cn.vimfung.luascriptcore.LuaExportType;

/**
 * TestObjectProxy
 * Created by vimfung on 2018/1/31.
 */
public class TestObjectProxy implements LuaExportType
{
    public static TestObj testObj(String name)
    {
        return TestObj.create(name);
    }
}
