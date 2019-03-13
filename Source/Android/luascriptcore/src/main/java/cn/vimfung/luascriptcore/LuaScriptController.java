package cn.vimfung.luascriptcore;

/**
 * 脚本控制器
 */
public class LuaScriptController extends LuaBaseObject
{
    /**
     * 创建上下文对象
     * @param nativeId  本地对象标识
     */
    protected LuaScriptController(int nativeId)
    {
        super(nativeId);
    }

    /**
     * 创建脚本控制器
     * @return 脚本控制器对象
     */
    public static LuaScriptController create()
    {
        return LuaNativeUtil.createScriptController();
    }

    /**
     * 设置脚本执行超时时间，如果脚本执行时间大于设置时间则会强制中断脚本，默认为0，
     */
    public void setTimeout(int timeout)
    {
        LuaNativeUtil.scriptControllerSetTimeout(this, timeout);
    }

    /**
     * 强制退出执行脚本
     */
    public void forceExit()
    {
        LuaNativeUtil.scriptControllerForceExit(this);
    }
}
