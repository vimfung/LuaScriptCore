package cn.vimfung.luascriptcore;

import java.lang.reflect.Array;
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

    public LuaValue ()
    {
        super();
        setNilValue();
    }

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

    public LuaValue(Integer value)
    {
        super();
        setIntValue(value);
    }

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

    public LuaValue(Double value)
    {
        super();
        setDoubValue(value);

    }

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

    public LuaValue(Boolean value)
    {
        super();
        setBoolValue(value);
    }

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

    public LuaValue(String value)
    {
        super();
        setStringValue(value);
    }

    protected LuaValue(int nativeId, String value)
    {
        super(nativeId);
        setStringValue(value);
    }

    private void setStringValue(String value)
    {
        _type = LuaValueType.String;
        _valueContainer = value;
    }

    public LuaValue (byte[] value)
    {
        super();
        setByteArrayValue(value);
    }

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

    public LuaValue (ArrayList value)
    {
        super();
        setArrayListValue(value);
    }

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

    public LuaValue (HashMap value)
    {
        super();
        setHasMapValue(value);
    }

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

    public int toInteger()
    {
        if (_type == LuaValueType.Integer)
        {
            return (Integer)_valueContainer;
        }

        return 0;
    }

    public double toNumber()
    {
        if (_type == LuaValueType.Number)
        {
            return (Double) _valueContainer;
        }

        return 0;
    }

    public boolean toBoolean()
    {
        if (_type == LuaValueType.Boolean)
        {
            return (Boolean) _valueContainer;
        }

        return false;
    }

    public String toString()
    {
        if (_type == LuaValueType.String)
        {
            return (String) _valueContainer;
        }

        return null;
    }

    public byte[] toByteArray()
    {
        if (_type == LuaValueType.Data)
        {
            return (byte[]) _valueContainer;
        }

        return null;
    }

    public ArrayList toArrayList()
    {
        if (_type == LuaValueType.Array)
        {
            return (ArrayList) _valueContainer;
        }
        return null;
    }

    public HashMap toHashMap()
    {
        if (_type == LuaValueType.Map)
        {
            return (HashMap) _valueContainer;
        }
        return null;
    }
}
