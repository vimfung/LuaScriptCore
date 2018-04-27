package cn.vimfung.luascriptcore.test;

import android.test.mock.MockContext;

import cn.vimfung.luascriptcore.LuaContext;

/**
 * Created by vimfung on 2018/4/27.
 */

public class Env
{
    private static Env _instance = new Env();
    public static Env sharedInstance ()
    {
        return _instance;
    }

    private LuaContext _context;
    private MockContext _mockContext;

    private Env()
    {
        _mockContext = new MockContext();
        _context = LuaContext.create(_mockContext);
    }

    public LuaContext getContext()
    {
        return _context;
    }
}
