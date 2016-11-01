package cn.vimfung.luascriptcore;

import android.util.Log;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * 模块类
 * Created by vimfung on 16/9/22.
 */
public abstract class LuaModule extends LuaBaseObject
{
    /**
     * 模块版本号, 子类可以重写该方法来设置自己的模块版本号
     * @return 版本号字符串
     */
    public static String version()
    {
        return null;
    }

    /**
     * 获取模块名称
     * @return 模块名称
     */
    public static String moduleName()
    {
        return null;
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

        //获取导出方法
        ArrayList<String> filterMethodNames = new ArrayList<String>();
        filterMethodNames.add("access$super");

        ArrayList<Method> exportMethods = new ArrayList<Method>();
        Method[] methods = moduleClass.getMethods();

        for (Method m : methods)
        {
            String methodName = m.getName();
            if (methodName.startsWith("_"))
            {
                continue;
            }

            int modifier = m.getModifiers();
            if (Modifier.isStatic(modifier)
                    && Modifier.isPublic(modifier)
                    && !Modifier.isAbstract(modifier)
                    && !filterMethodNames.contains(methodName))
            {

                //导出静态方法为模块的方法
                exportMethods.add(m);
            }
        }

        Method[] exportMethodsArr = exportMethods.toArray(new Method[0]);
        if (LuaNativeUtil.registerModule(context._nativeId, moduleName, moduleClass, exportMethodsArr))
        {
            LuaModule._setExportMethods(moduleClass, exportMethodsArr);
        }
    }

    /**
     * 保存导出方法的哈希表
     */
    static private HashMap<Class<? extends LuaModule>, HashMap<String, Method>> _exportMethods;

    /**
     * 导出方法
     * @param methods   方法集合
     */
    static protected void _setExportMethods(Class<? extends LuaModule> moduleClass, Method[] methods)
    {
        if (_exportMethods == null)
        {
            _exportMethods = new HashMap<Class<? extends LuaModule>, HashMap<String, Method>>();
        }

        if (!_exportMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> exportMethods = new HashMap<String, Method>();
            for (Method m : methods)
            {
                exportMethods.put(m.getName(), m);
            }
            _exportMethods.put(moduleClass, exportMethods);
        }
    }

    /**
     * 获取模块名称
     * @param moduleClass 模块类型
     * @return  模块名称
     */
    protected static String _getModuleName(Class<? extends LuaModule> moduleClass)
    {
        String modName = null;

        try {

            Method moduleNameMethod = moduleClass.getDeclaredMethod("moduleName");
            if (moduleNameMethod != null)
            {
                Object retValue = moduleNameMethod.invoke(moduleClass);
                if (retValue != null)
                {
                    modName = retValue.toString();
                }
            }

        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        if (modName == null)
        {
            modName = moduleClass.getSimpleName();
        }

        return modName;
    }

    /**
     * 获取导出方法
     * @param name  方法名称
     * @return  如果存在返回Method对象,否则返回null
     */
    static private Method _getExportMethod(Class<? extends LuaModule> moduleClass, String name)
    {
        if (_exportMethods != null && _exportMethods.containsKey(moduleClass))
        {
            HashMap<String, Method> methods = _exportMethods.get(moduleClass);
            return methods.get(name);
        }

        return null;
    }

    public LuaModule()
    {

    }


    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private static LuaValue _methodInvoke (Class<? extends LuaModule> moduleClass, String methodName, LuaValue[] arguments)
    {
        try
        {
            //将LuaValue数组转换为对象数组
            ArrayList argumentArray = new ArrayList();
            Method method =  LuaModule._getExportMethod(moduleClass, methodName);
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

            Object retValue = method.invoke(moduleClass, argumentArray.toArray());

            return new LuaValue(retValue);

        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        return new LuaValue();
    }
}
