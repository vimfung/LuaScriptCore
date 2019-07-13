package cn.vimfung.luascriptcore;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 导出类型管理器
 * Created by vimfung on 2017/11/17.
 */
class LuaExportTypeManager
{
    /**
     * 类型管理器
     */
    private static LuaExportTypeManager _manager = new LuaExportTypeManager();

    /**
     * 注册类方法集合
     */
    private static HashMap<Class<? extends LuaExportType>, HashMap<String, Method>> _regClassMethods = new HashMap<>();

    /**
     * 注册实例方法集合
     */
    private static HashMap<Class<? extends LuaExportType>, HashMap<String, Method>> _regInstanceMethods = new HashMap<>();

    /**
     * 注册字段集合
     */
    private static HashMap<Class<? extends LuaExportType>, HashMap<String, Field>> _regFieldMethods = new HashMap<>();

    /**
     * 获取默认管理器
     * @return 管理器对象
     */
    static LuaExportTypeManager getDefaultManager()
    {
        return _manager;
    }

    /**
     * 查找导出类型配置
     * @param t 导出类型
     * @return 配置信息
     */
    private LuaExportTypeConfig findExportTypeConfig(Class<? extends LuaExportType> t)
    {
        LuaExportTypeConfig exportTypeConfig = null;
        Annotation[] annotations = t.getDeclaredAnnotations();
        for (Annotation annotation : annotations)
        {
            if (annotation.annotationType().equals(LuaExportTypeConfig.class))
            {
                exportTypeConfig = (LuaExportTypeConfig) annotation;
                break;
            }
        }

        return exportTypeConfig;
    }

    /**
     * 是否为排除字段
     * @param field 字段信息
     * @return true 表示为排除字段，否则不是
     */
    private boolean isExcludeField(Field field)
    {
        LuaExclude exclude = null;
        Annotation[] annotations = field.getAnnotations();
        for (Annotation annotation : annotations)
        {
            if (annotation.annotationType().equals(LuaExclude.class))
            {
                return true;
            }
        }
        return false;
    }

    /**
     * 是否为排除方法
     * @param method 方法信息
     * @return true 表示为排除方法，否则不是
     */
    private boolean isExcludeMethod(Method method)
    {
        LuaExclude exclude = null;
        Annotation[] annotations = method.getAnnotations();
        for (Annotation annotation : annotations)
        {
            if (annotation.annotationType().equals(LuaExclude.class))
            {
                return true;
            }
        }
        return false;
    }

    /**
     * 是否为排除构造方法
     * @param constructor 构造方法
     * @return true表示排除方法，否则不是
     */
    private boolean isExcludeConstructor(Constructor constructor)
    {
        LuaExclude exclude = null;
        Annotation[] annotations = constructor.getAnnotations();
        for (Annotation annotation : annotations)
        {
            if (annotation.annotationType().equals(LuaExclude.class))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * 获取类型签名
     * @param type  类型
     * @return  签名
     */
    private String getTypeSignature(Class type)
    {
        if (int.class.isAssignableFrom(type))
        {
            return "i";
        }
        else if (short.class.isAssignableFrom(type))
        {
            return "s";
        }
        else if (char.class.isAssignableFrom(type))
        {
            return "c";
        }
        else if (long.class.isAssignableFrom(type))
        {
            return "q";
        }
        else if (boolean.class.isAssignableFrom(type))
        {
            return "B";
        }
        else if (float.class.isAssignableFrom(type))
        {
            return "f";
        }
        else if (double.class.isAssignableFrom(type))
        {
            return "d";
        }

        return "@";
    }

    /**
     * 根据传入参数获取类型构造器
     * @param targetType 目标类型
     * @param type  类型
     * @param arguments 参数
     * @return 构造器
     */
    private Constructor getConstructor(Class targetType, Class type, LuaValue[] arguments)
    {
        int targetMatchDegree = 0;
        Constructor targetConstructor = null;
        Constructor defaultConstructor = null;

        if (LuaExportType.class.isAssignableFrom(type))
        {
            Constructor[] constructors = type.getConstructors();

            for (Constructor constructor : constructors)
            {
                //判断是否为排除构造方法
                if(isExcludeConstructor(constructor))
                {
                    continue;
                }

                int matchDegree = 0;
                int index = 0;

                Class[] paramTypes = constructor.getParameterTypes();

                if (paramTypes.length == 0)
                {
                    //默认构造函数
                    defaultConstructor = constructor;
                }

                if (paramTypes.length != arguments.length)
                {
                    continue;
                }

                boolean hasMatch = true;
                for (Class paramType : paramTypes)
                {
                    LuaValue arg = arguments[index];

                    if (int.class.isAssignableFrom(paramType)
                            || long.class.isAssignableFrom(paramType)
                            || short.class.isAssignableFrom(paramType)
                            || Integer.class.isAssignableFrom(paramType)
                            || Short.class.isAssignableFrom(paramType)
                            || Long.class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.Integer || arg.valueType() == LuaValueType.Number)
                        {
                            matchDegree ++;
                        }
                    }
                    else if (float.class.isAssignableFrom(paramType)
                            || double.class.isAssignableFrom(paramType)
                            || Float.class.isAssignableFrom(paramType)
                            || Double.class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.Number)
                        {
                            matchDegree ++;
                        }
                    }
                    else if (boolean.class.isAssignableFrom(paramType)
                            || Boolean.class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.Boolean)
                        {
                            matchDegree ++;
                        }
                    }
                    else if (String.class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.String)
                        {
                            matchDegree ++;
                        }
                    }
                    else if (byte[].class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.Data || arg.valueType() == LuaValueType.String)
                        {
                            matchDegree ++;
                        }
                        else
                        {
                            hasMatch = false;
                        }
                    }
                    else if (paramType.isArray() || List.class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.Array)
                        {
                            matchDegree++;
                        }
                        else
                        {
                            hasMatch = false;
                        }
                    }
                    else if (Map.class.isAssignableFrom(paramType))
                    {
                        if (arg.valueType() == LuaValueType.Map)
                        {
                            matchDegree ++;
                        }
                        else
                        {
                            hasMatch = false;
                        }
                    }
                    else
                    {
                        Object obj = arg.toObject();
                        if (obj.getClass().isAssignableFrom(paramType))
                        {
                            matchDegree ++;
                        }
                        else
                        {
                            hasMatch = false;
                        }
                    }


                    if (!hasMatch)
                    {
                        break;
                    }

                    index ++;
                }

                if (hasMatch && matchDegree > targetMatchDegree)
                {
                    targetConstructor = constructor;
                    targetMatchDegree = matchDegree;
                }
            }

            if (targetConstructor == null)
            {
                //检测父类构造方法
                targetConstructor = getConstructor(targetType, type.getSuperclass(), arguments);
            }

            if (targetConstructor == null && targetType == type)
            {
                targetConstructor = defaultConstructor;
            }
        }

        return targetConstructor;
    }

    /**
     * 构造方法路由
     * @param context       上下文
     * @param type          类型
     * @param arguments     参数列表
     * @return  新构造对象
     */
    @SuppressWarnings("unchecked")
    LuaValue constructorMethodRoute(LuaContext context, Class<? extends  LuaExportType> type, LuaValue[] arguments)
    {
        Constructor constructor = getConstructor(type, type, arguments);
        if (constructor != null)
        {
            ArrayList<Object> args = new ArrayList<>();
            Class[] paramTypes = constructor.getParameterTypes();
            for (int i = 0; i < paramTypes.length; i++)
            {
                args.add(getArgValue(paramTypes[i], arguments[i]));
            }

            try
            {
                Object instance = constructor.newInstance(args.toArray(new Object[0]));

                return new LuaValue(instance);
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }

        return new LuaValue();
    }

    /**
     * 类方法路由
     * @param context       上下文
     * @param type          类型
     * @param arguments     参数列表
     * @return  返回值
     */
    @SuppressWarnings("unchecked")
    LuaValue classMethodRoute(LuaContext context, Class<? extends LuaExportType> type, String methodName, LuaValue[] arguments)
    {
        try
        {
            if (_regClassMethods.containsKey(type) && _regClassMethods.get(type).containsKey(methodName))
            {
                //将LuaValue数组转换为对象数组
                ArrayList argumentArray = new ArrayList();
                Method method =  _regClassMethods.get(type).get(methodName);
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
                    argumentArray.add(getArgValue(paramType, item));
                }

                Object retValue = method.invoke(type, argumentArray.toArray());

                return new LuaValue(retValue);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return new LuaValue();
    }

    /**
     * 实例方法路由
     * @param context 上下文对象
     * @param instance 实例对象
     * @param methodName 方法名称
     * @param arguments 参数列表
     * @return 返回值
     */
    @SuppressWarnings("unchecked")
    LuaValue instanceMethodRoute(LuaContext context, Object instance, String methodName, LuaValue[] arguments)
    {
        try
        {
            Class type = instance.getClass();
            if (_regInstanceMethods.containsKey(type) && _regInstanceMethods.get(type).containsKey(methodName))
            {
                //将LuaValue数组转换为对象数组
                ArrayList argumentArray = new ArrayList();
                Method method =  _regInstanceMethods.get(type).get(methodName);
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
                    argumentArray.add(getArgValue(paramType, item));

                }

                Object retValue = method.invoke(instance, argumentArray.toArray());

                return new LuaValue(retValue);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return new LuaValue();
    }

    /**
     * Getter方法路由
     * @param context       上下文对象
     * @param instance      实例对象
     * @param fieldName     字段名称
     * @return  返回值
     */
    LuaValue getterMethodRoute(LuaContext context, Object instance, String fieldName)
    {
        try
        {
            Class type = instance.getClass();
            if (_regFieldMethods.containsKey(type) && _regFieldMethods.get(type).containsKey(fieldName))
            {
                Field field =  _regFieldMethods.get(type).get(fieldName);
                if (field == null)
                {
                    return new LuaValue();
                }

                Object retValue = field.get(instance);

                return new LuaValue(retValue);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return new LuaValue();
    }

    /**
     * Setter方法路由
     * @param context       上下文对象
     * @param instance      实例对象
     * @param fieldName     字段名称
     * @param value         字段值
     */
    void setterMethodRoute(LuaContext context, Object instance, String fieldName, LuaValue value)
    {
        try
        {
            Class type = instance.getClass();
            if (_regFieldMethods.containsKey(type) && _regFieldMethods.get(type).containsKey(fieldName))
            {
                Field field =  _regFieldMethods.get(type).get(fieldName);
                if (field == null)
                {
                    return;
                }

                field.set(instance, getArgValue(field.getType(), value));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 获取参数值，主要将LuaValue转换为对应的参数值
     * @param argType 参数类型
     * @param value lua值
     * @return 参数值
     */
    private Object getArgValue(Class<?> argType, LuaValue value)
    {
        if (value == null || value.valueType() == LuaValueType.Nil)
        {
            return null;
        }

        if (int.class.isAssignableFrom(argType) || Integer.class.isAssignableFrom(argType))
        {
            return (int)value.toInteger();
        }
        else if (long.class.isAssignableFrom(argType) || Long.class.isAssignableFrom(argType))
        {
            return value.toInteger();
        }
        else if (float.class.isAssignableFrom(argType) || Float.class.isAssignableFrom(argType))
        {
            return (float)value.toDouble();
        }
        else if (double.class.isAssignableFrom(argType) || Double.class.isAssignableFrom(argType))
        {
            return value.toDouble();
        }
        else if (boolean.class.isAssignableFrom(argType) || Boolean.class.isAssignableFrom(argType))
        {
            return value.toBoolean();
        }
        else if (String.class.isAssignableFrom(argType))
        {
            return value.toString();
        }
        else if (byte[].class.isAssignableFrom(argType))
        {
            return value.toByteArray();
        }
        else if (ArrayList.class.isAssignableFrom(argType))
        {
            return value.toArrayList();
        }
        else if (HashMap.class.isAssignableFrom(argType))
        {
            return value.toHashMap();
        }
        else if (argType.isArray())
        {
            if (int[].class.isAssignableFrom(argType))
            {
                //转换数组中的Double型为整型
                ArrayList itemArr = value.toArrayList();
                int items[] = new int[itemArr.size()];
                for (int j = 0; j < itemArr.size(); j++)
                {
                    items[j] = ((Double)itemArr.get(j)).intValue();
                }
                return items;
            }
            else if (Integer[].class.isAssignableFrom(argType))
            {
                //转换数组中的Double型为整型
                ArrayList itemArr = value.toArrayList();
                Integer items[] = new Integer[itemArr.size()];
                for (int j = 0; j < itemArr.size(); j++)
                {
                    int item = ((Double)itemArr.get(j)).intValue();
                    items[j] = Integer.valueOf(item);
                }

                return items;
            }
            else if (Double[].class.isAssignableFrom(argType))
            {
                return value.toArrayList().toArray(new Double[0]);
            }
            else if (double[].class.isAssignableFrom(argType))
            {
                ArrayList itemArr = value.toArrayList();
                double items[] = new double[itemArr.size()];
                for (int j = 0; j < itemArr.size(); j++)
                {
                    items[j] = ((Double)itemArr.get(j)).doubleValue();
                }

                return items;
            }
            else if (Boolean[].class.isAssignableFrom(argType))
            {
                return value.toArrayList().toArray(new Boolean[0]);
            }
            else if (boolean[].class.isAssignableFrom(argType))
            {
                ArrayList itemArr = value.toArrayList();
                boolean items[] = new boolean[itemArr.size()];
                for (int j = 0; j < itemArr.size(); j++)
                {
                    items[j] = ((Boolean)itemArr.get(j)).booleanValue();
                }

                return items;
            }
            else
            {
                //当作Object数组处理
                return value.toArrayList().toArray();
            }
        }
        else
        {
            return value.toObject();
        }
    }

    /**
     * 导出类型
     * @param context 上下文对象
     * @param t 类型
     * @param st 父类型
     */
    @SuppressWarnings("unchecked")
    void exportType(LuaContext context, Class<? extends LuaExportType> t, Class<? extends  LuaExportType> st)
    {
        String alias = t.getSimpleName();
        String typeName = t.getName();
        LuaExportTypeConfig typeConfig = findExportTypeConfig(t);

        String baseTypeName = null;
        if (st != null)
        {
            baseTypeName = st.getName();
        }

        //获取导出属性、方法
        HashSet<String> exportFieldSet = new HashSet<>();
        HashMap<String, Field> exportFields = new HashMap<>();
        HashMap<String, Method> exportClassMethods = new HashMap<>();
        HashMap<String, Method> exportInstanceMethods = new HashMap<>();

        //导出字段
        Field[] fields = t.getFields();
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

            if (isExcludeField(field))
            {
                //为排除字段
                continue;
            }

            if (typeConfig != null && typeConfig.excludeExportFieldNames().length > 0)
            {
                boolean isExclude = false;
                for (String name : typeConfig.excludeExportFieldNames())
                {
                    if (name.equals(fieldName))
                    {
                        isExclude = true;
                        break;
                    }
                }

                if (isExclude)
                {
                    continue;
                }
            }

            exportFieldSet.add(String.format("%s_rw", fieldName));
            exportFields.put(fieldName, field);
        }

        //导出实例方法
        Method[] methods = t.getMethods();
        for (Method method : methods)
        {
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

            String methodName = method.getName();
            if (methodName.startsWith("_"))
            {
                //过滤下划线开头的字段
                continue;
            }

            if (methodName.equals("access$super"))
            {
                continue;
            }

            if (isExcludeMethod(method))
            {
                //为排除方法
                continue;
            }

            if (typeConfig != null && typeConfig.excludeExportInstanceMethodsNames().length > 0)
            {
                boolean isExclude = false;
                for (String name : typeConfig.excludeExportInstanceMethodsNames())
                {
                    if (name.equals(methodName))
                    {
                        isExclude = true;
                        break;
                    }
                }

                if (isExclude)
                {
                    continue;
                }
            }

            //构造签名
            StringBuilder signatureStringBuilder = new StringBuilder();
            Class[] paramTypes = method.getParameterTypes();
            for (Class paramType : paramTypes)
            {
                signatureStringBuilder.append(getTypeSignature(paramType));
            }

            //导出方法
            exportInstanceMethods.put(String.format("%s_%s", methodName, signatureStringBuilder.toString()), method);
        }

        //导出类方法
        methods = t.getMethods();
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
                    && !methodName.equals("access$super")
                    && !isExcludeMethod(method))
            {
                if (typeConfig != null && typeConfig.excludeExportClassMethodNames().length > 0)
                {
                    boolean isExclude = false;
                    for (String name : typeConfig.excludeExportClassMethodNames())
                    {
                        if (name.equals(methodName))
                        {
                            isExclude = true;
                            break;
                        }
                    }

                    if (isExclude)
                    {
                        continue;
                    }
                }

                //构造签名
                StringBuilder signatureStringBuilder = new StringBuilder();
                Class[] paramTypes = method.getParameterTypes();
                for (Class paramType : paramTypes)
                {
                    signatureStringBuilder.append(getTypeSignature(paramType));
                }

                //导出静态方法为模块的方法
                exportClassMethods.put(String.format("%s_%s", methodName, signatureStringBuilder.toString()), method);
            }
        }

        String[] exportClassMethodsArr = exportClassMethods.keySet().toArray(new String[0]);
        String[] exportInstanceMethodsArr = exportInstanceMethods.keySet().toArray(new String[0]);
        String[] exportFieldArr = exportFieldSet.toArray(new String[0]);

        if (LuaNativeUtil.registerType(
                context,
                alias,
                typeName,
                baseTypeName,
                t,
                exportFieldArr,
                exportInstanceMethodsArr,
                exportClassMethodsArr))
        {
            //导出成功，写入导出方法和字段
            if (exportClassMethods.size() > 0)
            {
                _regClassMethods.put(t, exportClassMethods);
            }
            if (exportInstanceMethods.size() > 0)
            {
                _regInstanceMethods.put(t, exportInstanceMethods);
            }
            if (exportFields.size() > 0)
            {
                _regFieldMethods.put(t, exportFields);
            }
        }
    }
}
