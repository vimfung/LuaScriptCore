package cn.vimfung.luascriptcore.sample;

import java.util.HashMap;

import cn.vimfung.luascriptcore.LuaExportType;
import cn.vimfung.luascriptcore.LuaExportTypeConfig;

/**
 * Created by vimfung on 2017/4/17.
 */
@LuaExportTypeConfig()
public class NativeData implements LuaExportType
{
    public String dataId;

    private HashMap<String, String> _data = new HashMap<String, String>();

    public static Person createPerson()
    {
        return new Person();
    }

    public void setData(String key, String value)
    {
        _data.put(key, value);
    }

    public String getData(String key)
    {
        return _data.get(key);
    }
}
