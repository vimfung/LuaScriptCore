package cn.vimfung.luascriptcore;

public class LuaThread extends LuaBaseObject
{
    private LuaContext _context;
    private LuaMethodHandler _handler;
    private boolean _finished;

    /**
     * 初始化
     * @param nativeId 原生对象标志
     */
    protected LuaThread(int nativeId)
    {
        super(nativeId);
    }

    /**
     * 创建线程
     * @param context 上下文对象
     * @param handler 线程处理器
     * @return 线程对象
     */
    public static LuaThread create(LuaContext context, LuaMethodHandler handler)
    {
        LuaThread thread = LuaNativeUtil.createThread(context);
        thread._context = context;
        thread._handler = handler;

        return thread;
    }

    /**
     * 恢复线程
     * @param arguments 请求参数列表
     */
    public void resume(LuaValue[] arguments)
    {
        LuaNativeUtil.resumeThread(this, arguments);
    }

    /**
     * 挂起线程
     * @param resultValue 返回值
     */
    public void yield(LuaValue resultValue)
    {
        LuaNativeUtil.yieldThread(this, resultValue);
    }

    /**
     * 获取上下文对象
     * @return 上下文对象
     */
    public LuaContext getContext()
    {
        return _context;
    }

    /**
     * 是否完成
     * @return true 表示执行完成，false 表示尚未完成
     */
    public boolean hasFinished()
    {
        return _finished;
    }

    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private LuaValue methodInvoke (String methodName, LuaValue[] arguments)
    {
        _handler.onExecute(arguments);
        return null;

    }
}
