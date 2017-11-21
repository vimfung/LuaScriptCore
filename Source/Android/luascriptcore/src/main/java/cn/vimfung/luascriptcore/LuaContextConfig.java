package cn.vimfung.luascriptcore;

/**
 * 上下文配置
 * Created by vimfung on 2017/11/15.
 */
public class LuaContextConfig
{
    /**
     * 是否收到导入类型，如果设置为true，则所以继承LuaExportType的类型在lua中需要使用nativeType来进行导入。
     */
    public boolean manualImportClassEnabled = false;

    /**
     * 默认配置
     */
    private  static LuaContextConfig _config = new LuaContextConfig ();

    /**
     * 获取默认上下文配置信息
     * @return 上下文配置
     */
    public static LuaContextConfig defaultConfig()
    {
        return _config;
    }
}
