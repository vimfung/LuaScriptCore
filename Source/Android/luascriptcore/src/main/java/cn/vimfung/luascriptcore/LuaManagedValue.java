package cn.vimfung.luascriptcore;

import android.util.Log;

/**
 * 管理值对象，用于将LuaValue的值所指向的Lua对象生命周期维持在LuaManagedValue释放后进行。
 * Created by vimfung on 2017/5/24.
 */
public class LuaManagedValue extends LuaBaseObject
{
    private LuaValue _source;
    private LuaContext _context;

    /**
     * 初始化对象
     * @param value     原始值对象
     * @param context   上下文对象
     */
    public LuaManagedValue(LuaValue value, LuaContext context)
    {
        _source = value;
        _context = context;

        context.retainValue(_source);
    }

    /**
     * 获取源值
     * @return  值对象
     */
    public LuaValue getSource()
    {
        return _source;
    }

    @Override
    protected void finalize() throws Throwable
    {
        _context.releaseValue(_source);
        super.finalize();
    }
}
