package cn.vimfung.luascriptcore;

/**
 * Lua方法对象
 * Created by vimfung on 16/11/1.
 */
public class LuaFunction extends LuaBaseObject
{
    private LuaContext _context;

    /**
     * 创建方法对象
     * @param nativeId  本地对象标识
     * @param context 上下文对象
     */
    protected LuaFunction(int nativeId, LuaContext context)
    {
        super(nativeId);
        _context = context;
    }

    /**
     * 调用方法
     * @param arguments 参数列表
     * @return  返回值
     */
    public LuaValue invoke(LuaValue[] arguments)
    {
        return invoke(arguments, null);
    }

    public LuaValue invoke(LuaValue[] arguments, LuaScriptController scriptController)
    {
        return LuaNativeUtil.invokeFunction(_context, this, arguments, scriptController);
    }

    /**
     * 获取上下文对象
     * @return 上下文对象
     */
    public LuaContext getContext()
    {
        return _context;
    }
}
