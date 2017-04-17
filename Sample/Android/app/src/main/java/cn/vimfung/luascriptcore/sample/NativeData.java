package cn.vimfung.luascriptcore.sample;

import java.util.HashMap;

/**
 * Created by vimfung on 2017/4/17.
 */

public class NativeData
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
