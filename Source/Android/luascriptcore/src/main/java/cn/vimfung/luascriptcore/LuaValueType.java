package cn.vimfung.luascriptcore;

/**
 * LuaValue的类型
 */
public enum LuaValueType
{
    Nil (0),            //空值
    Number (1),         //数值
    Boolean (2),        //布尔
    String (3),         //字符串
    Array (4),          //数组
    Map (5),            //字典
    Pointer (6),        //指针
    Object (7),         //对象
    Integer (8),        //整型
    Data (9),           //二进制数组
    Function (10),      //方法
    Tuple(11);          //元组，用于返回值

    private int _value;

    LuaValueType(int i)
    {
        _value = i;
    }

    public int value()
    {
        return _value;
    }
}
