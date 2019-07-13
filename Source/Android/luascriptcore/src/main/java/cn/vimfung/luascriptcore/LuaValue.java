package cn.vimfung.luascriptcore;

import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Lua值对象
 * Created by vimfung on 16/8/23.
 */
public class LuaValue extends LuaBaseObject
{
    private Object _valueContainer;
    private LuaValueType _type;
    private LuaContext _context;    //从JNI层回来的对象会带上这个字段
    private String _tableId;           //Map和Array类型下的Table标识

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
        setLongValue(Long.valueOf(value));
    }

    /**
     * 初始化一个整型值的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 整型值
     */
    protected LuaValue(int nativeId, Integer value)
    {
        super(nativeId);
        setLongValue(Long.valueOf(value));
    }

    /**
     * 初始化一个长整型的LuaValue对象
     * @param value 长整型值
     */
    public LuaValue(Long value)
    {
        super();
        setLongValue(value);
    }

    /**
     * 设置长整型值
     * @param value 长整型值
     */
    private void setLongValue(Long value)
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
     * 初始化一个字节数组的LuaValue对象
     * @param value 字节数组
     */
    public LuaValue (Byte[] value)
    {
        super();
        setByteArrayValue(value);
    }

    /**
     * 设置二进制数组
     * @param value 二进制数组
     */
    private void setByteArrayValue(Byte[] value)
    {
        _type = LuaValueType.Data;
        _valueContainer = value;
    }

    /**
     * 初始化一个数组的LuaValue对象
     * @param value 数组
     */
    public LuaValue (List<?> value)
    {
        super();
        setArrayListValue(value);
    }

    /**
     * 初始化一个数组的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 数组
     */
    protected LuaValue (int nativeId, List<?> value, String tableId)
    {
        super(nativeId);
        setArrayListValue(value);
        _tableId = tableId;
    }

    /**
     * 设置数组列表
     * @param value 数组列表值
     */
    private void setArrayListValue(List<?> value)
    {
        _type = LuaValueType.Array;
        _valueContainer = value;
    }

    /**
     * 初始化一个哈希表的LuaValue对象
     * @param value 哈希表
     */
    public LuaValue (Map<?, ?> value)
    {
        super();
        setHasMapValue(value);
    }

    /**
     * 初始化一个哈希表的LuaValue对象
     * @param nativeId 本地对象标识
     * @param value 哈希表
     */
    protected LuaValue (int nativeId, Map<?, ?> value, String tableId)
    {
        super(nativeId);
        setHasMapValue(value);
        _tableId = tableId;
    }

    /**
     * 设置HashMap
     * @param value HashMap值
     */
    private void setHasMapValue (Map<?, ?> value)
    {
        _type = LuaValueType.Map;
        _valueContainer = value;
    }

    /**
     * 初始化指针对象的LuaValue对象
     * @param value 指针对象
     */
    public LuaValue (LuaPointer value)
    {
        super();
        setPointerValue(value);
    }

    /**
     * 初始化指针对象的LuaValue对象
     * @param nativeId  本地标识对象
     * @param value 指针对象
     */
    protected LuaValue (int nativeId, LuaPointer value)
    {
        super(nativeId);
        setPointerValue(value);
    }

    /**
     * 设置指针对象
     * @param value 指针对象
     */
    private void setPointerValue (LuaPointer value)
    {
        _type = LuaValueType.Pointer;
        _valueContainer = value;
    }

    /**
     * 初始化值对象
     * @param value 方法对象
     */
    public LuaValue (LuaFunction value)
    {
        super();
        setFunctionValue(value);
    }

    /**
     * 初始化值对象
     * @param nativeId  本地标识
     * @param value     方法对象
     */
    protected  LuaValue (int nativeId, LuaFunction value)
    {
        super(nativeId);
        setFunctionValue(value);
    }

    /**
     * 设置方法对象
     * @param value 方法对象
     */
    private void setFunctionValue (LuaFunction value)
    {
        _type = LuaValueType.Function;
        _valueContainer = value;
    }

    /**
     * 设置元组
     * @param value 元组对象
     */
    public LuaValue (LuaTuple value)
    {
        super();
        setTupleValue(value);
    }

    /**
     * 初始化值对象
     * @param nativeId  本地标识
     * @param value     元组对象
     */
    protected  LuaValue (int nativeId, LuaTuple value)
    {
        super(nativeId);
        setTupleValue(value);
    }

    /**
     * 设置元组值
     * @param value 元组对象
     */
    private void setTupleValue (LuaTuple value)
    {
        _type = LuaValueType.Tuple;
        _valueContainer = value;
    }

    /**
     * 初始化值对象
     * @param value 对象
     */
    public LuaValue (Object value)
    {
        super();
        setObjectValue(value);
    }

    /**
     * 初始化值对象
     * @param nativeId  本地标识
     * @param value 对象
     */
    protected LuaValue (int nativeId, Object value)
    {
        super(nativeId);
        setObjectValue(value);
    }

    /**
     * 设置对象值
     * @param value 对象
     */
    private void setObjectValue (Object value)
    {
        if (value != null)
        {
            if (value instanceof Character)
            {
                setLongValue(Long.valueOf((Character)value));
            }
            else if (value instanceof Byte)
            {
                setLongValue(Long.valueOf((Byte)value));
            }
            else if (value instanceof Short)
            {
               setLongValue(Long.valueOf((Short)value));
            }
            else if (value instanceof Integer)
            {
                setLongValue(Long.valueOf((Integer)value));
            }
            else if (value instanceof Long)
            {
                setLongValue((Long) value);
            }
            else  if (value instanceof Float)
            {
                setDoubValue(((Float) value).doubleValue());
            }
            else if (value instanceof Double)
            {
                setDoubValue((Double)value);
            }
            else if (value instanceof Boolean)
            {
                setBoolValue((Boolean)value);
            }
            else if (value instanceof String)
            {
                setStringValue(value.toString());
            }
            else if (value instanceof byte[])
            {
                setByteArrayValue((byte[]) value);
            }
            else if (value instanceof List<?>)
            {
                setArrayListValue((List<?>)value);
            }
            else if (value instanceof Map<?, ?>)
            {
                setHasMapValue((Map<?, ?>)value);
            }
            else if (value instanceof LuaPointer)
            {
                setPointerValue((LuaPointer) value);
            }
            else if (value instanceof LuaFunction)
            {
                setFunctionValue((LuaFunction) value);
            }
            else if (value instanceof LuaTuple)
            {
                setTupleValue((LuaTuple) value);
            }
            else
            {
                _type = LuaValueType.Object;
                _valueContainer = value;
            }
        }
        else
        {
            _type = LuaValueType.Nil;
            _valueContainer = null;
        }
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
    public long toInteger()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof Number)
            {
                return ((Number) _valueContainer).longValue();
            }
            else if (_valueContainer instanceof String)
            {
                return Integer.parseInt((String) _valueContainer);
            }
        }

        return 0;
    }

    /**
     * 转换为浮点数
     * @return 浮点数
     */
    public double toDouble()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof Number)
            {
                return ((Number) _valueContainer).doubleValue();
            }
            else if (_valueContainer instanceof String)
            {
                return Double.parseDouble((String) _valueContainer);
            }
        }

        return 0.0;
    }

    /**
     * 转换为布尔值
     * @return 布尔值
     */
    public boolean toBoolean()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof Boolean)
            {
                return (Boolean) _valueContainer;
            }
            else if (_valueContainer instanceof String)
            {
                return Boolean.parseBoolean((String) _valueContainer);
            }
        }

        return false;
    }

    /**
     * 转换为字符串
     * @return  字符串
     */
    public String toString()
    {
        if (_valueContainer != null)
        {
            return _valueContainer.toString();
        }

        return "";
    }

    /**
     * 转换为二进制数组
     * @return  二进制数组
     */
    public byte[] toByteArray()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof byte[])
            {
                return (byte[]) _valueContainer;
            }
            else if (_valueContainer instanceof Byte[])
            {
                final Byte[] srcBytes = (Byte[])_valueContainer;
                byte[] bytes = new byte[srcBytes.length];
                for (int i = 0; i < srcBytes.length; i++)
                {
                    bytes[i]= srcBytes[i];
                }

                return bytes;
            }
            else if (_valueContainer instanceof String)
            {
                return ((String) _valueContainer).getBytes();
            }
        }

        return null;
    }

    /**
     * 转换为数组列表
     * @return 数组列表
     */
    public ArrayList toArrayList()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof ArrayList)
            {
                return (ArrayList) _valueContainer;
            }
        }

        return null;
    }

    /**
     * 转换为列表
     * @return 列表对象
     */
    public List<?> toList()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof List<?>)
            {
                return (List<?>) _valueContainer;
            }
        }

        return null;
    }

    /**
     * 转换为哈希表
     * @return  哈希表
     */
    public HashMap toHashMap()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof HashMap)
            {
                return (HashMap) _valueContainer;
            }
        }

        return null;
    }

    /**
     * 转换为字典
     * @return 字典对象
     */
    public Map<?, ?> toMap()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof Map<?, ?>)
            {
                return (Map<?, ?>) _valueContainer;
            }
        }

        return null;
    }

    /**
     * 转换为指针对象
     * @return  指针对象
     */
    public LuaPointer toPointer()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof LuaPointer)
            {
                return (LuaPointer) _valueContainer;
            }
        }

        return null;
    }

    /**
     * 转换为方法对象
     * @return 方法对象
     */
    public LuaFunction toFunction()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof LuaFunction)
            {
                return (LuaFunction) _valueContainer;
            }
        }

        return null;
    }

    /**
     * 转换为元组对象
     * @return  方法对象
     */
    public LuaTuple toTuple()
    {
        if (_valueContainer != null)
        {
            if (_valueContainer instanceof LuaTuple)
            {
                return (LuaTuple) _valueContainer;
            }
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

    /**
     * 设置对象对象
     * @param keyPath  键名路径，如：key1.key2.key3
     * @param object 对象
     */
    public void setObject(String keyPath, Object object)
    {
        if (valueType() == LuaValueType.Map)
        {
            _valueContainer = LuaNativeUtil.luaValueSetObject(_context, this, keyPath, new LuaValue(object));
        }
    }

    /**
     * 获取table标识
     * @return table标识
     */
    protected String getTableId()
    {
        return _tableId;
    }
}
