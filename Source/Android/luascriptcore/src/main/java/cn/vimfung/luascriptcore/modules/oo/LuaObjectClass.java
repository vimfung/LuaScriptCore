package cn.vimfung.luascriptcore.modules.oo;

import android.util.ArraySet;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaModule;
import cn.vimfung.luascriptcore.LuaNativeUtil;
import cn.vimfung.luascriptcore.LuaValue;
import dalvik.system.DexClassLoader;

/**
 * 面向对象基类
 * Created by vimfung on 16/9/27.
 */
public class LuaObjectClass extends LuaModule
{
    public static String version ()
    {
        return "1.0.0";
    }

    public static String moduleName ()
    {
        return "Object";
    }

    /**
     * 注册模块
     * @param context   Lua上下文对象
     * @param moduleClass    模块类型
     */
    public static void _register(LuaContext context, Class<? extends LuaModule> moduleClass)
    {
        String moduleName = LuaModule._getModuleName(moduleClass);
        if (context.isModuleRegisted(moduleName))
        {
            return;
        }

        //先判断注册的类型的父类是否已经注册
        Class superClass = moduleClass.getSuperclass();
        if (superClass != LuaModule.class && !context.isModuleRegisted(LuaModule._getModuleName(superClass)))
        {
            context.registerModule(superClass);
        }


        ArrayList<Field> exportFields = new ArrayList<Field>();
        ArrayList<String> filterMethodNames = new ArrayList<String>();
        ArrayList<Method> exportClassMethods = new ArrayList<Method>();
        ArrayList<Method> exportInstanceMethods = new ArrayList<Method>();

        filterMethodNames.add("access$super");
        filterMethodNames.add("createInstance");
        filterMethodNames.add("getContext");

        //导出字段
        Field[] fields = moduleClass.getDeclaredFields();
        for (Field field : fields)
        {
            int modifier = field.getModifiers();
            if (Modifier.isStatic(modifier))
            {
                continue;
            }

            if (!Modifier.isPublic(modifier))
            {
                continue;
            }

            if (Modifier.isAbstract(modifier))
            {
                continue;
            }

            String fieldName = field.getName();
            if (fieldName.startsWith("_"))
            {
                //过滤下划线开头的字段
                continue;
            }

            exportFields.add(field);
        }

        //导出实例方法
        Method[] methods = moduleClass.getDeclaredMethods();
        for (Method method : methods)
        {
            String methodName = method.getName();

            if (methodName.startsWith("_"))
            {
                //过滤下划线开头的字段
                continue;
            }

            int modifier = method.getModifiers();
            if (Modifier.isStatic(modifier))
            {
                continue;
            }

            if (!Modifier.isPublic(modifier))
            {
                continue;
            }

            if (Modifier.isAbstract(modifier))
            {
                continue;
            }

            if (filterMethodNames.contains(methodName))
            {
                continue;
            }

            //导出方法
            exportInstanceMethods.add(method);
        }

        //导出类方法
        methods = moduleClass.getDeclaredMethods();
        for (Method method : methods)
        {
            String methodName = method.getName();

            if (methodName.startsWith("_"))
            {
                continue;
            }

            int modifier = method.getModifiers();
            if (Modifier.isStatic(modifier)
                    && Modifier.isPublic(modifier)
                    && !Modifier.isAbstract(modifier)
                    && !filterMethodNames.contains(methodName))
            {

                //导出静态方法为模块的方法
                exportClassMethods.add(method);
            }
        }

        Method[] exportClassMethodsArr = exportClassMethods.toArray(new Method[0]);
        Method[] exportInstanceMethodsArr = exportInstanceMethods.toArray(new Method[0]);
        Field[] exportFieldArr = exportFields.toArray(new Field[0]);

        String superClassName = (moduleClass == LuaObjectClass.class ? null : LuaModule._getModuleName(superClass));
        if (LuaNativeUtil.registerClass(
                context,
                moduleName,
                superClassName,
                (Class<? extends LuaObjectClass>) moduleClass,
                exportFieldArr,
                exportInstanceMethodsArr,
                exportClassMethodsArr))
        {
            LuaModule._setExportMethods(moduleClass, exportClassMethodsArr);
            LuaObjectClass._setExportInstanceMethods((Class<? extends LuaObjectClass>) moduleClass, exportInstanceMethodsArr);
        }
    }

    /**
     * 保存导出方法的哈希表
     */
    static private HashMap<Class<? extends LuaObjectClass>, HashMap<String, Method>> _exportInstanceMethods;

    /**
     * 导出实例方法
     * @param moduleClass 模块类型
     * @param methods   方法集合
     */
    static private void _setExportInstanceMethods(Class<? extends LuaObjectClass> moduleClass, Method[] methods)
    {
        if (_exportInstanceMethods == null)
        {
            _exportInstanceMethods = new HashMap<Class<? extends LuaObjectClass>, HashMap<String, Method>>();
        }

        if (!_exportInstanceMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> exportMethods = new HashMap<String, Method>();

            for (Method m : methods)
            {
                exportMethods.put(m.getName(), m);
            }
            _exportInstanceMethods.put(moduleClass, exportMethods);
        }
    }

    /**
     * 获取导出实例方法
     * @param moduleClass   模块类型
     * @param methodName    方法名称
     * @return  方法对象
     */
    static private Method _getExportInstanceMethod(Class<? extends  LuaObjectClass> moduleClass, String methodName)
    {
        if (_exportInstanceMethods != null && _exportInstanceMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> methods = _exportInstanceMethods.get(moduleClass);
            return methods.get(methodName);
        }

        return null;
    }

    private LuaContext _context;

    /**
     * 获取Lua上下文对象
     * @return 上下文对象
     */
    public LuaContext getContext()
    {
        return _context;
    }

    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private LuaValue _instanceMethodInvoke (String methodName, LuaValue[] arguments)
    {
        try
        {
            //将LuaValue数组转换为对象数组
            ArrayList argumentArray = new ArrayList();
            Method method =  LuaObjectClass._getExportInstanceMethod(this.getClass(), methodName);
            if (method == null)
            {
                return new LuaValue();
            }

            Class<?>[] types = method.getParameterTypes();
            for (int i = 0; i < types.length; i++)
            {
                LuaValue item = null;
                if (arguments.length > i)
                {
                    item = arguments[i];
                }
                else
                {
                    item = new LuaValue();
                }

                Class<?> paramType = types[i];
                if (paramType.isAssignableFrom(int.class))
                {
                    argumentArray.add(item.toInteger());
                }
                else if (paramType.isAssignableFrom(double.class))
                {
                    argumentArray.add(item.toDouble());
                }
                else if (paramType.isAssignableFrom(boolean.class))
                {
                    argumentArray.add(item.toBoolean());
                }
                else if (paramType.isAssignableFrom(String.class))
                {
                    argumentArray.add(item.toString());
                }
                else if (paramType.isAssignableFrom(byte[].class))
                {
                    argumentArray.add(item.toByteArray());
                }
                else if (paramType.isAssignableFrom(ArrayList.class))
                {
                    argumentArray.add(item.toArrayList());
                }
                else if (paramType.isAssignableFrom(HashMap.class))
                {
                    argumentArray.add(item.toHashMap());
                }
                else if (paramType.isArray())
                {
                    if (paramType.isAssignableFrom(int[].class))
                    {
                        //转换数组中的Double型为整型
                        ArrayList itemArr = item.toArrayList();
                        int items[] = new int[itemArr.size()];
                        for (int j = 0; j < itemArr.size(); j++)
                        {
                            items[j] = ((Double)itemArr.get(j)).intValue();
                        }
                        argumentArray.add(items);
                    }
                    else if (paramType.isAssignableFrom(Integer[].class))
                    {
                        //转换数组中的Double型为整型
                        ArrayList itemArr = item.toArrayList();
                        Integer items[] = new Integer[itemArr.size()];
                        for (int j = 0; j < itemArr.size(); j++)
                        {
                            int value = ((Double)itemArr.get(j)).intValue();
                            items[j] = new Integer(value);
                        }

                        argumentArray.add(items);
                    }
                    else if (paramType.isAssignableFrom(Double[].class))
                    {
                        argumentArray.add(item.toArrayList().toArray(new Double[0]));
                    }
                    else if (paramType.isAssignableFrom(double[].class))
                    {
                        ArrayList itemArr = item.toArrayList();
                        double items[] = new double[itemArr.size()];
                        for (int j = 0; j < itemArr.size(); j++)
                        {
                            items[j] = ((Double)itemArr.get(j)).doubleValue();
                        }

                        argumentArray.add(items);
                    }
                    else if (paramType.isAssignableFrom(Boolean[].class))
                    {
                        argumentArray.add(item.toArrayList().toArray(new Boolean[0]));
                    }
                    else if (paramType.isAssignableFrom(boolean[].class))
                    {
                        ArrayList itemArr = item.toArrayList();
                        boolean items[] = new boolean[itemArr.size()];
                        for (int j = 0; j < itemArr.size(); j++)
                        {
                            items[j] = ((Boolean)itemArr.get(j)).booleanValue();
                        }

                        argumentArray.add(items);
                    }
                    else
                    {
                        //当作Object数组处理
                        argumentArray.add(item.toArrayList().toArray());
                    }
                }
                else
                {
                    argumentArray.add(item.toObject());
                }

            }

            Object retValue = method.invoke(this, argumentArray.toArray());

            return new LuaValue(retValue);

        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        return new LuaValue();
    }

    /**
     * 设置字段
     * @param name   字段名称
     * @param value  字段值
     */
    private void _setField(String name, LuaValue value)
    {
        try
        {
            Field field = this.getClass().getField(name);
            Class<?> fieldType = field.getType();

            if (fieldType.isAssignableFrom(int.class) || fieldType.isAssignableFrom(Integer.class))
            {
                field.set(this, value.toInteger());
            }
            else if (fieldType.isAssignableFrom(double.class) || fieldType.isAssignableFrom(Double.class))
            {
                field.set(this, value.toDouble());
            }
            else if (fieldType.isAssignableFrom(boolean.class) || fieldType.isAssignableFrom(Boolean.class))
            {
                field.set(this, value.toBoolean());
            }
            else
            {
                field.set(this, value.toObject());
            }

        }
        catch (NoSuchFieldException e)
        {
            e.printStackTrace();
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 获取字段
     * @param name  字段名称
     * @return 字段值
     */
    private LuaValue _getField(String name)
    {
        try
        {
            Field field = this.getClass().getField(name);
            return new LuaValue(field.get(this));
        }
        catch (NoSuchFieldException e)
        {
            return new LuaValue();
        }
        catch (IllegalAccessException e)
        {
            return new LuaValue();
        }
    }
}
