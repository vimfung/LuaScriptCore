package cn.vimfung.luascriptcore.modules.oo;

import cn.vimfung.luascriptcore.LuaValue;

/**
 * 类实例对象,该对象指示一个Lua中的类实例对象
 * Created by vimfung on 16/9/27.
 */
public class LuaClassInstance
{
    private Class<? extends LuaObjectClass> _ownerClass;
    private LuaObjectClass _nativeObject;

    /**
     * 获取当前实例所属类
     * @return 类型
     */
    public Class<? extends LuaObjectClass> getOwnerClass ()
    {
        return _ownerClass;
    }

    /**
     * 获取当前实例的本地对象
     * @return  类实例本地对象
     */
    public LuaObjectClass getNativeObject ()
    {
        return _nativeObject;
    }

    /**
     * 设置字段值
     * @param name  字段名称
     * @param value 字段值
     */
    public void setField(String name, LuaValue value)
    {

    }

    /**
     * 获取字段值
     * @param name  字段名称
     * @return  字段值
     */
    public LuaValue getField(String name)
    {
        return null;
    }

    /**
     * 调用实例方法
     * @param name  方法名称
     * @param arguments 参数列表
     * @return  返回值
     */
    public LuaValue callMethod(String name, LuaValue[] arguments)
    {
        return null;
    }

}
