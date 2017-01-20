package cn.vimfung.luascriptcore;

import java.util.ArrayList;

/**
 * Created by vimfung on 17/1/19.
 * 元组，用于多个返回值的返回时使用
 */
public class LuaTuple extends LuaBaseObject
{
    private ArrayList<LuaValue> _returnValues;

    public LuaTuple ()
    {
        _returnValues = new ArrayList<LuaValue>();
    }

    /**
     * 元素数量
     * @return 数量
     */
    public int count()
    {
        return _returnValues.size();
    }

    /**
     * 添加返回值
     * @param value 返回值
     */
    public void addReturnValue(Object value)
    {
        _returnValues.add(new LuaValue(value));
    }

    /**
     * 获取返回值
     * @param index 返回值索引
     * @return  返回值
     */
    public Object getReturnValueByIndex(int index)
    {
        return _returnValues.get(index).toObject();
    }
}
