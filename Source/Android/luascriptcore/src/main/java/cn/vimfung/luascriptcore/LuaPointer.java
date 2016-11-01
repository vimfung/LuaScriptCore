package cn.vimfung.luascriptcore;

/**
 * Created by vimfung on 16/10/31.
 *
 * Lua的指针类型
 */
public class LuaPointer extends LuaBaseObject
{
    /**
     * 创建指针对象
     * @param nativeId  本地对象标识
     */
    protected LuaPointer(int nativeId)
    {
        super(nativeId);
    }
}
