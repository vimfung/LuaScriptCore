package cn.vimfung.luascriptcore;

import java.util.ArrayList;

/**
 * 方法处理器
 * Created by vimfung on 16/9/11.
 */
public interface LuaMethodHandler
{
    /**
     * 执行方法时调用
     * @param arguments 传入参数
     * @return 返回值
     */
    public LuaValue onExecute(LuaValue[] arguments);
}
