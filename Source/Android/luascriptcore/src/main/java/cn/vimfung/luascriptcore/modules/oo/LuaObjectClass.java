package cn.vimfung.luascriptcore.modules.oo;

import android.util.ArraySet;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.HashSet;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaModule;
import cn.vimfung.luascriptcore.LuaNativeUtil;

/**
 * 面向对象基类
 * Created by vimfung on 16/9/27.
 */
public class LuaObjectClass extends LuaModule
{
    /**
     * 实例缓存池,主要用于与lua中的对象保持相同的生命周期而设定,创建时放入池中,当gc回收时从池中移除。
     */
    private static HashSet<LuaObjectClass> _instancePool = new HashSet<LuaObjectClass>();
    private static LuaClassInstance _currentInstance = null;

    public static String version ()
    {
        return "1.0.0";
    }

    protected static String moduleName ()
    {
        return "Object";
    }

    /**
     * 放入实例
     * @param instance  实例对象
     */
    private static void putInstance(LuaObjectClass instance)
    {
        _instancePool.add(instance);
    }

    /**
     * 移除实例
     * @param instance  实例对象
     */
    private static void removeInstance(LuaObjectClass instance)
    {
        _instancePool.remove(instance);
    }

    /**
     * 获取当前实例
     * @return  实例对象
     */
    public static LuaClassInstance getCurrentInstance()
    {
        return _currentInstance;
    }

    /**
     * 实例对象描述,子类重写该方法返回对象描述
     * @param instance  实例对象
     * @return 描述字符串
     */
    protected String instanceDescription(LuaClassInstance instance)
    {
        String className = LuaModule.getModuleName(this.getClass());
        if (className != null)
        {
            return String.format("[%s object]", className);
        }

        return "Invalid Object";
    }

    /**
     * 实例初始化时触发,子类重写该方法进行对象初始化
     * @param instance 实例对象
     */
    protected void instanceInitialize(LuaClassInstance instance)
    {
        Log.v("lsc", "======== instance initialize");
    }

    /**
     * 实例释放时触发,子类重写该方法进行对象释放
     * @param instance  实例对象
     */
    protected void instanceUninitialize(LuaClassInstance instance)
    {
        Log.v("lsc", "======== instance uninitialize");
    }

    /**
     * 注册模块
     * @param context   Lua上下文对象
     * @param moduleName    模块名称
     */
    protected static LuaModule register(LuaContext context, String moduleName, Class<? extends LuaModule> moduleClass)
    {
        Log.v("lsc", "register class");

        //先判断注册的类型的父类是否已经注册
        Class superClass = moduleClass.getSuperclass();
        if (superClass != LuaModule.class && !context.isModuleRegisted(LuaModule.getModuleName(superClass)))
        {
            context.registerModule(superClass);
        }

        ArrayList<Field> exportFields = new ArrayList<Field>();
        ArrayList<String> filterMethodNames = new ArrayList<String>();
        ArrayList<Method> exportMethods = new ArrayList<Method>();

        filterMethodNames.add("instanceDescription");
        filterMethodNames.add("instanceInitialize");
        filterMethodNames.add("instanceUninitialize");

        Field[] fields = moduleClass.getDeclaredFields();
        for (Field field : fields)
        {
            int modifier = field.getModifiers();
            if (Modifier.isStatic(modifier))
            {
                Log.v("lsc", "is static");
                continue;
            }

            if (!Modifier.isPublic(modifier))
            {
                Log.v("lsc", "is not public");
                continue;
            }

            if (Modifier.isAbstract(modifier))
            {
                Log.v("lsc", "is abstract");
                continue;
            }

            exportFields.add(field);
        }

        Method[] methods = moduleClass.getDeclaredMethods();
        for (Method method : methods)
        {
            Log.v("lsc", "================");
            String methodName = method.getName();
            Log.v("lsc", methodName);

            int modifier = method.getModifiers();
            if (Modifier.isStatic(modifier))
            {
                Log.v("lsc", "is static");
                continue;
            }

            if (!Modifier.isPublic(modifier))
            {
                Log.v("lsc", "is not public");
                continue;
            }

            if (Modifier.isAbstract(modifier))
            {
                Log.v("lsc", "is abstract");
                continue;
            }

            if (filterMethodNames.contains(methodName))
            {
                Log.v("lsc", "filter method");
                continue;
            }

            //导出方法
            exportMethods.add(method);
        }

        Method[] exportMethodsArr = exportMethods.toArray(new Method[0]);
        Field[] exportFieldArr = exportFields.toArray(new Field[0]);

        LuaObjectClass objectClass =  LuaNativeUtil.registerClass(
                context,
                moduleName,
                (moduleClass == LuaObjectClass.class ? null : LuaModule.getModuleName(superClass)),
                (Class<? extends LuaObjectClass>) moduleClass,
                exportFieldArr,
                exportMethodsArr);

        if (objectClass != null)
        {
            objectClass._setExportMethods(exportMethodsArr);
        }

        return objectClass;
    }

}
