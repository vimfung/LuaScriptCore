package cn.vimfung.luascriptcore;

public enum LuaValueType
{
    Nil (0),
    Number (1),
    Boolean (2),
    String (3),
    Array (4),
    Map (5),
    Integer (8),
    Data (9);

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
