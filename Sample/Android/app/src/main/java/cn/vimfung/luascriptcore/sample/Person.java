package cn.vimfung.luascriptcore.sample;

import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaFunction;
import cn.vimfung.luascriptcore.LuaTuple;
import cn.vimfung.luascriptcore.LuaValue;
import cn.vimfung.luascriptcore.modules.oo.LuaObjectClass;

/**
 * 人类
 * Created by vimfung on 16/10/12.
 */
public class Person extends LuaObjectClass
{
    public String name;
    public void speak()
    {
        Log.v("luascriptcore", String.format("%s speak", name));
    }

    public void walk()
    {
        Log.v("luascriptcore", String.format("%s walk", name));
    }

    public static void printPersonName(Person p)
    {
        Log.v("lsc", p.name);
    }

    public static Person createPerson()
    {
        return new Person();
    }

    public static LuaTuple test(String a, String b)
    {
        LuaTuple tuple = new LuaTuple();
        tuple.addReturnValue(a);
        tuple.addReturnValue(b);

        return tuple;
    }

    public static void log (String msg)
    {
        Log.v("lsc===", msg);
    }

    private static LuaValue _func = null;

    public static void retainHandler(LuaFunction handler)
    {
        _func = new LuaValue(handler);
        Env.defaultContext().retainValue(_func);
    }

    public static void releaseHandler(LuaFunction handler)
    {
        Env.defaultContext().releaseValue(_func);
    }

    public static void callHandler()
    {
        if (_func != null)
        {
            _func.toFunction().invoke(null);
        }
    }
}
