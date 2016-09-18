package cn.vimfung.luascriptcore;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * Lua值对象
 * Created by vimfung on 16/8/23.
 */
public class LuaValue extends LuaBaseObject
{
    private Object _valueContainer;
    private LuaValueType _type;

    /**
     * 初始化一个空值的LuaValue对象
     */
    public LuaValue ()
    {
        super();
        setNilValue();
    }

    /**
     * 初始化一个控制的LuaValue对象
     * @param nativeId  本地对象标识
     */
    protected LuaValue(int nativeId)
    {
        super(nativeId);
        setNilValue();
    }

    /**
     * 设置空值
     */
    private void setNilValue ()
    {
        _type = LuaValueType.Nil;
        _valueContainer = null;
    }

    /**
     * 初始化一个整型值的LuaValue对象
     * @param value 整型值
     */
    public LuaValue(Integer value)
    {
        super();
        setIntValue(value);
    }

    /**
     * 初始化一个整型值的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 整型值
     */
    protected LuaValue(int nativeId, Integer value)
    {
        super(nativeId);
        setIntValue(value);
    }

    /**
     * 设置整型值
     * @param value 整型值
     */
    private void setIntValue (Integer value)
    {
        _type = LuaValueType.Integer;
        _valueContainer = value;
    }

    /**
     * 初始化一个浮点值的LuaValue对象
     * @param value 浮点数值
     */
    public LuaValue(Double value)
    {
        super();
        setDoubValue(value);

    }

    /**
     * 初始化一个浮点值的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 浮点数值
     */
    protected LuaValue(int nativeId, Double value)
    {
        super(nativeId);
        setDoubValue(value);
    }

    /**
     * 设置双精度浮点型数值
     * @param value 浮点数值
     */
    private void setDoubValue(Double value)
    {
        _type = LuaValueType.Number;
        _valueContainer = value;
    }

    /**
     * 初始化一个布尔值的LuaValue对象
     * @param value 布尔值
     */
    public LuaValue(Boolean value)
    {
        super();
        setBoolValue(value);
    }

    /**
     * 初始化一个布尔值的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 布尔值
     */
    protected LuaValue(int nativeId, Boolean value)
    {
        super(nativeId);
        setBoolValue(value);
    }

    /**
     * 设置布尔值
     * @param value 布尔值
     */
    private void setBoolValue(Boolean value)
    {
        _type = LuaValueType.Boolean;
        _valueContainer = value;
    }

    /**
     * 初始化一个字符串的LuaValue对象
     * @param value 字符串
     */
    public LuaValue(String value)
    {
        super();
        setStringValue(value);
    }

    /**
     * 初始化一个字符串的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 字符串
     */
    protected LuaValue(int nativeId, String value)
    {
        super(nativeId);
        setStringValue(value);
    }

    /**
     * 设置字符串
     * @param value 字符串
     */
    private void setStringValue(String value)
    {
        _type = LuaValueType.String;
        _valueContainer = value;
    }

    /**
     * 初始化一个字节数组的LuaValue对象
     * @param value 字节数组
     */
    public LuaValue (byte[] value)
    {
        super();
        setByteArrayValue(value);
    }

    /**
     * 初始化一个字节数组的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 字节数组
     */
    protected LuaValue(int nativeId, byte[] value)
    {
        super(nativeId);
        setByteArrayValue(value);
    }

    /**
     * 设置二进制数组
     * @param value 二进制数组值
     */
    private void setByteArrayValue(byte[] value)
    {
        _type = LuaValueType.Data;
        _valueContainer = value;
    }

    /**
     * 初始化一个数组的LuaValue对象
     * @param value 数组
     */
    public LuaValue (ArrayList value)
    {
        super();
        setArrayListValue(value);
    }

    /**
     * 初始化一个数组的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 数组
     */
    protected LuaValue (int nativeId, ArrayList value)
    {
        super(nativeId);
        setArrayListValue(value);
    }

    /**
     * 设置数组列表
     * @param value 数组列表值
     */
    private void setArrayListValue(ArrayList value)
    {
        _type = LuaValueType.Array;
        _valueContainer = value;
    }

    /**
     * 初始化一个哈希表的LuaValue对象
     * @param value 哈希表
     */
    public LuaValue (HashMap value)
    {
        super();
        setHasMapValue(value);
    }

    /**
     * 初始化一个哈希表的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 哈希表
     */
    protected LuaValue (int nativeId, HashMap value)
    {
        super(nativeId);
        setHasMapValue(value);
    }

    /**
     * 设置HashMap
     * @param value HashMap值
     */
    private void setHasMapValue (HashMap value)
    {
        _type = LuaValueType.Map;
        _valueContainer = value;
    }

    /**
     * 获取值类型
     * @return  类型
     */
    public LuaValueType valueType ()
    {
        return _type;
    }

    /**
     * 转换为整数
     * @return 整数
     */
    public int toInteger()
    {
        if (_valueContainer instanceof Number)
        {
            return ((Number)_valueContainer).intValue();
        }
        else if (_valueContainer instanceof String)
        {
            return Integer.parseInt((String) _valueContainer);
        }

        return 0;
    }

    /**
     * 转换为浮点数
     * @return 浮点数
     */
    public double toDouble()
    {
        if (_valueContainer instanceof Number)
        {
            return ((Number)_valueContainer).doubleValue();
        }
        else if (_valueContainer instanceof String)
        {
            return Double.parseDouble((String) _valueContainer);
        }

        return 0.0;
    }

    /**
     * 转换为布尔值
     * @return 布尔值
     */
    public boolean toBoolean()
    {
        if (_valueContainer instanceof Boolean)
        {
            return (Boolean) _valueContainer;
        }
        else if (_valueContainer instanceof String)
        {
            return Boolean.parseBoolean((String) _valueContainer);
        }

        return false;
    }

    /**
     * 转换为字符串
     * @return  字符串
     */
    public String toString()
    {
        return _valueContainer.toString();
    }

    /**
     * 转换为二进制数组
     * @return  二进制数组
     */
    public byte[] toByteArray()
    {
        if (_type == LuaValueType.Data)
        {
            return (byte[]) _valueContainer;
        }

        return null;
    }

    /**
     * 转换为数组列表
     * @return 数组列表
     */
    public ArrayList toArrayList()
    {
        if (_type == LuaValueType.Array)
        {
            return (ArrayList) _valueContainer;
        }
        return null;
    }

    /**
     * 转换为哈希表
     * @return  哈希表
     */
    public HashMap toHashMap()
    {
        if (_type == LuaValueType.Map)
        {
            return (HashMap) _valueContainer;
        }
        return null;
    }

    /**
     * 转换为对象
     * @return  对象
     */
    public Object toObject()
    {
        return _valueContainer;
    }
}
