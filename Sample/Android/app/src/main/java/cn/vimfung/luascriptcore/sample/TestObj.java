package cn.vimfung.luascriptcore.sample;

/**
 * TestObj
 * Created by vimfung on 2018/1/31.
 */
public class TestObj
{
    private String name;

    public static TestObj create(String name)
    {
        return new TestObj().res(name);
    }

    private TestObj res(String name)
    {
        this.name = name;
        return this;
    }
}
