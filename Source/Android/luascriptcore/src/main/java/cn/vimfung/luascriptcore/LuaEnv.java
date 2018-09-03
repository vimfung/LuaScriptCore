package cn.vimfung.luascriptcore;

import android.annotation.SuppressLint;
import android.content.Context;

/**
 * 环境信息
 */
public final class LuaEnv
{
    /**
     * 共享环境对象
     */
    @SuppressLint("StaticFieldLeak")
    private static final LuaEnv _env = new LuaEnv();
    public static LuaEnv defaultEnv ()
    {
        return _env;
    }

    /**
     * Android应用上下文
     */
    private Context _androidApplicationContext = null;

    /**
     * 设置Android应用上下文
     * @param context 上下文
     */
    public void setAndroidApplicationContext(Context context)
    {
        _androidApplicationContext = context.getApplicationContext();
    }

    /**
     * 获取Android应用上下文
     * @return 上下文
     */
    public Context getAndroidApplicationContext()
    {
        return _androidApplicationContext;
    }
}
