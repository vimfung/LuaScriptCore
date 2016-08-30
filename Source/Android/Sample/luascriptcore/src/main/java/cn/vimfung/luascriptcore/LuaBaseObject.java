package cn.vimfung.luascriptcore;

/**
 * 基础类型
 * Created by vimfung on 16/8/29.
 */
public class LuaBaseObject
{
    /**
     * 本地标识
     */
    protected int _nativeId;

    /**
     * 创建基础对象
     */
    public LuaBaseObject ()
    {
        _nativeId = 0;
    }

    /**
     * 创建基础对象
     * @param nativeId  本地标识
     */
    protected LuaBaseObject(int nativeId)
    {
        _nativeId = nativeId;
    }

    @Override
    protected void finalize() throws Throwable
    {
        if (_nativeId > 0)
        {
            //释放本地对象
            LuaNativeUtil.releaseNativeObject(_nativeId);
        }
        super.finalize();
    }
}
