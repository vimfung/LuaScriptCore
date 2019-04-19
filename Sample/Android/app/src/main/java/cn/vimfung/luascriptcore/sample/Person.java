package cn.vimfung.luascriptcore.sample;

import android.support.annotation.NonNull;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaExclude;
import cn.vimfung.luascriptcore.LuaExportType;
import cn.vimfung.luascriptcore.LuaExportTypeConfig;
import cn.vimfung.luascriptcore.LuaFunction;
import cn.vimfung.luascriptcore.LuaManagedValue;
import cn.vimfung.luascriptcore.LuaTuple;
import cn.vimfung.luascriptcore.LuaValue;

/**
 * 人类
 * Created by vimfung on 16/10/12.
 */
public class Person implements LuaExportType
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

    public Person ()
    {

    }

    public List<int[]> getIntArray()
    {
        ArrayList<int[]> val = new ArrayList<>();

        int[] arr1 = new int[] {1, 2, 3};
        val.add(arr1);

        int[] arr2 = new int[] {4, 5, 6};
        val.add(arr2);

        return val;
    }

    public List<Integer[]> getIntegerArray()
    {
        ArrayList<Integer[]> val = new ArrayList<>();

        Integer item1 = new Integer(7);
        Integer item2 = new Integer(8);
        Integer item3 = new Integer(9);
        Integer[] arr1 = new Integer[] {item1, item2, item3};
        val.add(arr1);

        return val;
    }

    public List<long[]> getLongArray()
    {
        ArrayList<long[]> val = new ArrayList<>();

        long[] arr1 = new long[] {10, 11, 12};
        val.add(arr1);

        return val;
    }

    public List<Long[]> getLong2Array()
    {
        ArrayList<Long[]> val = new ArrayList<>();

        Long item1 = new Long(13);
        Long item2 = new Long(14);
        Long item3 = new Long(15);
        Long[] arr1 = new Long[] {item1, item2, item3};
        val.add(arr1);

        return val;
    }

    public List<float[]> getFloatArray()
    {
        ArrayList<float[]> val = new ArrayList<>();

        float[] arr1 = new float[] {1.1f, 1.2f, 1.3f};
        val.add(arr1);

        return val;
    }

    public List<Float[]> getFloat2Array()
    {
        ArrayList<Float[]> val = new ArrayList<>();

        Float item1 = new Float(1.4f);
        Float item2 = new Float(1.5f);
        Float item3 = new Float(1.6f);
        Float[] arr1 = new Float[] {item1, item2, item3};
        val.add(arr1);

        return val;
    }

    public List<double[]> getDoubleArray()
    {
        ArrayList<double[]> val = new ArrayList<>();

        double[] arr1 = new double[] {1.7f, 1.8f, 1.9f};
        val.add(arr1);

        return val;
    }

    public List<Double[]> getDouble2Array()
    {
        ArrayList<Double[]> val = new ArrayList<>();

        Double item1 = new Double(10.4f);
        Double item2 = new Double(10.5f);
        Double item3 = new Double(10.6f);
        Double[] arr1 = new Double[] {item1, item2, item3};
        val.add(arr1);

        return val;
    }

    public List<boolean[]> getBooleanArray()
    {
        ArrayList<boolean[]> val = new ArrayList<>();

        boolean[] arr1 = new boolean[] {true, false, true};
        val.add(arr1);

        return val;
    }

    public List<Boolean[]> getBoolean2Array()
    {
        ArrayList<Boolean[]> val = new ArrayList<>();

        Boolean item1 = new Boolean(false);
        Boolean item2 = new Boolean(true);
        Boolean item3 = new Boolean(false);
        Boolean[] arr1 = new Boolean[] {item1, item2, item3};
        val.add(arr1);

        return val;
    }

    public float callFloatMethod(float value)
    {
        return value + 1024;
    }

    public float callFloat2Method(Float value)
    {
        return value + 1024;
    }

    public Person (String name)
    {
        this.name = name;
    }


    public static Person createPerson()
    {
        return new Person();
    }

    public static Person createPersonError()
    {
        Env.defaultContext().raiseException("can't create person");
        return new Person();
    }

    public static void action(TestObj testObj)
    {
        Log.v("lsc", testObj.toString());
    }

    public static Object createObj()
    {
        Person obj = new Person();
        obj.name = "vim";
        return obj;
    }

    public static void CheckObj(Object obj)
    {
        Person p = (Person)obj;
        p.speak();
//        Log.v("lsc", obj.toString());
    }

    public static LuaTuple test(String a, String b)
    {
        LuaTuple tuple = new LuaTuple();
        tuple.addReturnValue(a);
        tuple.addReturnValue(b);

        return tuple;
    }

    public void log (String msg)
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

    private static LuaManagedValue _func2 = null;

    public static void retainHandler2(LuaFunction handler)
    {
        _func2 = new LuaManagedValue(new LuaValue(handler), Env.defaultContext());
    }

    public static void releaseHandler2(LuaFunction handler)
    {
        _func2 = null;
    }

    public static void callHandler2()
    {
        if (_func2 != null) {
            _func2.getSource().toFunction().invoke(null);
        }
    }

    public byte[] getBuffer()
    {
        return new byte[512];
    }
}
