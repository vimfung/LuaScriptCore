package cn.vimfung.luascriptcore;

/**
 * Lua异常处理器
 */
public interface LuaExceptionHandler
{
    /**
     * 发生异常是触发
     * @param message 错误信息
     */
    public void onException(String message);
}
