package cn.vimfung.luascriptcore.modules.oo;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaModule;
import cn.vimfung.luascriptcore.LuaNativeUtil;
import cn.vimfung.luascriptcore.LuaValue;

/**
 * 类型导入
 * Created by vimfung on 17/3/27.
 */
public class LuaClassImport extends LuaModule
{
    public static String version ()
    {
        return "1.0.0";
    }

    public static String moduleName ()
    {
        return "ClassImport";
    }

    /**
     * 注册模块
     * @param context   Lua上下文对象
     * @param moduleClass    模块类型
     */
    public static void _register(LuaContext context, Class<? extends LuaModule> moduleClass)
    {
        String moduleName = LuaModule._getModuleName(moduleClass);
        LuaNativeUtil.registerClassImport(context, moduleName);
    }

    /**
     * 保存导出方法的哈希表
     */
    static private HashMap<Class, HashMap<String, Method>> _exportClassMethods;
    static private HashMap<Class, HashMap<String, Method>> _exportInstanceMethods;

    /**
     * 导出类方法
     * @param methods   方法集合
     */
    static private void _setExportClassMethods(Class moduleClass, Method[] methods)
    {
        if (_exportClassMethods == null)
        {
            _exportClassMethods = new HashMap<Class, HashMap<String, Method>>();
        }

        if (!_exportClassMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> exportMethods = new HashMap<String, Method>();
            for (Method m : methods)
            {
                exportMethods.put(m.getName(), m);
            }
            _exportClassMethods.put(moduleClass, exportMethods);
        }
    }

    /**
     * 获取导出类方法
     * @param name  方法名称
     * @return  如果存在返回Method对象,否则返回null
     */
    static private Method _getExportClassMethod(Class moduleClass, String name)
    {
        if (_exportClassMethods != null && _exportClassMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> methods = _exportClassMethods.get(moduleClass);
            return methods.get(name);
        }

        return null;
    }

    /**
     * 导出类方法
     * @param methods   方法集合
     */
    static private void _setExportInstanceMethods(Class moduleClass, Method[] methods)
    {
        if (_exportInstanceMethods == null)
        {
            _exportInstanceMethods = new HashMap<Class, HashMap<String, Method>>();
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
     * 获取导出类方法
     * @param name  方法名称
     * @return  如果存在返回Method对象,否则返回null
     */
    static private Method _getExportInstanceMethod(Class moduleClass, String name)
    {
        if (_exportInstanceMethods != null && _exportInstanceMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> methods = _exportInstanceMethods.get(moduleClass);
            return methods.get(name);
        }

        return null;
    }


    /**
     * 设置包含的类型列表
     * @param context 上下文对象
     * @param classes 类型集合
     */
    public static void setInculdesClasses(LuaContext context, List<Class> classes)
    {
        LuaNativeUtil.setInculdesClasses(context, classes);
    }

    /**
     * 获取导出类方法
     * @param cls 类型
     * @return 方法名称列表
     */
    private static String[] _getExportClassMethod(Class cls)
    {
        Method[] methods = cls.getMethods();
        ArrayList<String> methodNames = new ArrayList<String>();

        for (Method m : methods)
        {
            int modifiers = m.getModifiers();
            if (Modifier.isStatic(modifiers)
                    && Modifier.isPublic(modifiers)
                    && !Modifier.isAbstract(modifiers)
                    && !Modifier.isNative(modifiers))
            {
                methodNames.add(m.getName());
            }
        }

        _setExportClassMethods(cls, methods);

        return methodNames.toArray(new String[0]);
    }

    /**
     * 获取导出实例方法
     * @param cls 类型
     * @return 方法名称列表
     */
    private static String[] _getExportInstanceMethod(Class cls)
    {
        Method[] methods = cls.getMethods();
        ArrayList<String> methodNames = new ArrayList<String>();

        for (Method m : methods)
        {
            int modifiers = m.getModifiers();
            if (!Modifier.isStatic(modifiers)
                    && Modifier.isPublic(modifiers)
                    && !Modifier.isAbstract(modifiers)
                    && !Modifier.isNative(modifiers))
            {
                methodNames.add(m.getName());
            }
        }

        _setExportInstanceMethods(cls, methods);

        return methodNames.toArray(new String[0]);
    }

    /**
     * 获取导出字段
     * @param cls 类型
     * @return 字段名称列表
     */
    private static String[] _getExportFieldMethod(Class cls)
    {
        Field[] fields = cls.getFields();
        ArrayList<String> fieldNames = new ArrayList<String>();

        for (Field f : fields)
        {
            int modifiers = f.getModifiers();
            if (!Modifier.isStatic(modifiers)
                    && Modifier.isPublic(modifiers)
                    && !Modifier.isAbstract(modifiers)
                    && !Modifier.isNative(modifiers))
            {
                fieldNames.add(f.getName());
            }
        }

        return fieldNames.toArray(new String[0]);
    }

    /**
     * 设置字段
     * @param name   字段名称
     * @param value  字段值
     */
    private static void _setField(Class type, Object instance, String name, LuaValue value)
    {
        try
        {
            Field field = type.getField(name);
            Class<?> fieldType = field.getType();

            if (fieldType.isAssignableFrom(int.class) || fieldType.isAssignableFrom(Integer.class))
            {
                field.set(instance, value.toInteger());
            }
            else if (fieldType.isAssignableFrom(double.class) || fieldType.isAssignableFrom(Double.class))
            {
                field.set(instance, value.toDouble());
            }
            else if (fieldType.isAssignableFrom(boolean.class) || fieldType.isAssignableFrom(Boolean.class))
            {
                field.set(instance, value.toBoolean());
            }
            else
            {
                field.set(instance, value.toObject());
            }

        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 获取字段
     * @param name  字段名称
     * @return 字段值
     */
    private static LuaValue _getField(Class type, Object instance, String name)
    {
        try
        {
            Field field = type.getField(name);
            return new LuaValue(field.get(instance));
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

    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private static LuaValue _classMethodInvoke (Class type, String methodName, LuaValue[] arguments)
    {
        try
        {
            //将LuaValue数组转换为对象数组
            ArrayList argumentArray = new ArrayList();

            Method method =  _getExportClassMethod(type, methodName);
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

            Object retValue = method.invoke(type, argumentArray.toArray());

            return new LuaValue(retValue);

        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return new LuaValue();
    }

    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private static LuaValue _instanceMethodInvoke (Class type, Object instance, String methodName, LuaValue[] arguments)
    {
        try
        {
            //将LuaValue数组转换为对象数组
            ArrayList argumentArray = new ArrayList();

            //从子类到父类逐个类型查找导出方法集合中是否存在指定方法
            Method method = _getExportInstanceMethod(type, methodName);
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

            Object retValue = method.invoke(instance, argumentArray.toArray());

            return new LuaValue(retValue);

        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return new LuaValue();
    }
}
