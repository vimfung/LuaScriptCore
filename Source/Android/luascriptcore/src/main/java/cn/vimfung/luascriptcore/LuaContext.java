package cn.vimfung.luascriptcore;

import android.app.Application;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Path;
import android.os.Debug;
import android.os.Environment;
import android.provider.ContactsContract;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.lang.annotation.Annotation;
import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.ServiceLoader;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import dalvik.system.DexFile;

import static android.os.Environment.MEDIA_MOUNTED;
//import java.util.Objects;

/**
 * Lua上下文对象
 * Created by vimfung on 16/8/22.
 */
public class LuaContext extends LuaBaseObject
{
    private Context _context;
    private LuaExceptionHandler _exceptionHandler;
    private HashMap<String, LuaMethodHandler> _methods;

    /**
     * 建立Lua目录标识
     */
    private static boolean _isSetupLuaFolder = false;

    /**
     * 注册类型
     */
    private static ArrayList<Class<? extends LuaExportType>> _regTypes = null;

    /**
     * 排除类型规则
     */
    private static String[] _defaultExcludeClassesRules = {
            "^android[.]support$",
            "^android[.]support[.].+",
            "^com[.]android$",
            "^com[.]android[.].+"
    };

    /**
     * 查找导出的类型
     *
     * @param context 上下文对象
     */
    @SuppressWarnings("unchecked")
    private static void findRegTypes (Context context)
    {
        if (_regTypes == null)
        {
            _regTypes = new ArrayList<Class<? extends LuaExportType>>();
            try
            {
                //获取排除类型规则
                ArrayList<String> excludeRules = new ArrayList<String>();
                excludeRules.addAll(Arrays.asList(_defaultExcludeClassesRules));
                excludeRules.addAll(excludeClassesRules);

                String packageCodePath = context.getPackageCodePath();
                DexFile df = new DexFile(packageCodePath);
                for (Enumeration<String> iter = df.entries(); iter.hasMoreElements();)
                {
                    String className = iter.nextElement();

                    boolean isExcludeClass = false;
                    for (String patternStr : excludeRules)
                    {
                        Pattern pattern = Pattern.compile(patternStr);
                        Matcher matcher = pattern.matcher(className);
                        if (matcher.matches())
                        {
                            //匹配
                            isExcludeClass = true;
                            break;
                        }
                    }

                    if (!isExcludeClass)
                    {
                        Log.d("luascriptcore", String.format("found Class = %s", className));
                        try
                        {
                            Class type = Class.forName(className);
                            if (!type.isInterface() && LuaExportType.class.isAssignableFrom(type))
                            {
                                //非接口
                                Log.d("luascriptcore", String.format("Register type %s", type.getName()));
                                _regTypes.add(type);
                            }
                        }
                        catch (ClassNotFoundException e)
                        {
                            Log.d("luascriptcore", e.getMessage());
                        }
                        catch (Exception e)
                        {
                            Log.w("luascriptcore", e.getMessage());
                        }

                    }
                }
            }
            catch (IOException e)
            {
                Log.d("luascriptcore", e.getMessage());
            }
        }
    }

    /**
     * 建立Lua目录结构
     */
    private void setupLuaFolder()
    {
        if (!_isSetupLuaFolder)
        {
            copyLuaFileFromAssets("");
            _isSetupLuaFolder = true;
        }
    }

    /**
     * 获取缓存目录路径
     * @return  缓存目录
     */
    private File getCacheDir()
    {
        File appCacheDir = null;

        int perm = _context.checkCallingOrSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if (MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) &&  perm == PackageManager.PERMISSION_GRANTED)
        {
            appCacheDir = _context.getExternalCacheDir();
        }

        if (appCacheDir == null)
        {

            appCacheDir = _context.getCacheDir();
        }

        return appCacheDir;
    }

    /**
     * 从资源中拷贝Lua文件
     * @param path 资源文件路径
     */
    private void copyLuaFileFromAssets(String path)
    {
        try
        {
            String[] paths = _context.getAssets().list(path);
            if (paths.length > 0)
            {
                //为目录，继续迭代查找
                for (String subPath : paths)
                {
                    copyLuaFileFromAssets(String.format("%s%s/", path, subPath));
                }
            }
            else
            {
                //为文件，拷贝到临时目录
                String fileName = path.substring(0, path.length() - 1);
                if (fileName.toLowerCase().endsWith(".lua"))
                {
                    //为lua文件，进行拷贝
                    ByteArrayOutputStream dataStream = new ByteArrayOutputStream();
                    try
                    {
                        String filePath = String.format("%s/lua/%s", getCacheDir(), fileName);
                        File file = new File(filePath);
                        File parentFile = file.getParentFile();
                        if (!parentFile.exists())
                        {
                            parentFile.mkdirs();
                        }

                        readAssetFileContent(fileName, dataStream);
                        //直接覆盖缓存路径中的文件，保证以assets中的内容为最新。
                        writeToFile(file, dataStream);
                        dataStream.close();

                    }
                    catch (IOException e)
                    {
                        e.printStackTrace();
                    }
                }
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 导出类型
     */
    private void exportTypes()
    {
        for (Class<? extends LuaExportType> t : _regTypes)
        {
            LuaExportTypeManager.getDefaultManager().exportType(this, t);
        }
    }

    /**
     * 创建上下文对象
     * @param nativeId  本地对象标识
     */
    protected LuaContext(int nativeId)
    {
        super(nativeId);

        this._methods = new HashMap<>();

        //导出类型
        exportTypes();
    }

    public static ArrayList<String> excludeClassesRules = new ArrayList<>();

    /**
     * 创建上下文对象
     *
     * @param context  应用上下文对象
     * @return Lua上下文对象
     */
    public static LuaContext create(Context context)
    {
        //初始化注册类型
        findRegTypes(context);

        LuaContext luaContext = LuaNativeUtil.createContext();
        luaContext._context = context;

        File cacheDir = new File (String.format("%s/lua", luaContext.getCacheDir()));
        if (!cacheDir.exists())
        {
            cacheDir.mkdirs();
        }
        luaContext.addSearchPath(cacheDir.toString());

        return luaContext;
    }

    /**
     * 异常时触发
     * @param handler 异常处理器
     */
    public void onException (LuaExceptionHandler handler)
    {
        _exceptionHandler = handler;
        LuaNativeUtil.catchException(this, _exceptionHandler != null);
    }

    /**
     * 添加搜索路径, 对于需要引用不同目录下的lua文件,需要设置其搜索路径,否则会导致无法找到脚本而运行出错
     * @param path  搜索路径
     */
    public void addSearchPath (String path)
    {
        LuaNativeUtil.addSearchPath(_nativeId, path + "/?.lua");
    }

    /**
     * 设置全局变量
     * @param name  名称
     * @param value 值
     */
    public void setGlobal(String name, LuaValue value)
    {
        LuaNativeUtil.setGlobal(_nativeId, name, value);
    }

    /**
     * 获取全局变量
     * @param name 名称
     * @return 值
     */
    public LuaValue getGlobal(String name)
    {
        return LuaNativeUtil.getGlobal(_nativeId, name);
    }

    /**
     * 释放Lua层的变量引用，使其内存管理权交回Lua。
     * 注：判断value能否被释放取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
     * 即：LuaValue val1 = new LuaValue(obj1);与LuaValue val2 = new LuaValue(obj1);传入方法中效果相同。
     *
     * @param value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。
     */
    public void retainValue(LuaValue value)
    {
        LuaNativeUtil.retainValue(this, value);
    }

    /**
     * 释放Lua层的变量引用，使其内存管理权交回Lua。
     * 注：判断value能否被释放取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
     * 即：LSCValue *val1 = [LSCValue objectValue:obj1]与LSCValue *val2 = [LSCValue objectValue:obj1]传入方法中效果相同。
     *
     * @param value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。
     */
    public void releaseValue(LuaValue value)
    {
        LuaNativeUtil.releaseValue(this, value);
    }

    /**
     * 解析Lua脚本
     * @param script  脚本
     * @return 执行后返回的值
     */
    public LuaValue evalScript (String script)
    {
        //拷贝资源包中的所有lua文件到临时目录中
        //fixed : lua字符串包含对资源包中文件的引用
        setupLuaFolder();

        return LuaNativeUtil.evalScript(_nativeId, script);
    }

    /**
     * 解析Lua脚本文件
     * @param filePath  Lua文件路径
     * @return  执行后返回值
     */
    public LuaValue evalScriptFromFile (String filePath)
    {
        //拷贝资源包中的所有lua文件到临时目录中
        //fixed : 其他路径下的lua文件包含对资源包中文件的引用
        setupLuaFolder();

        LuaValue retValue = null;

        String AssetsPathPrefix = "/android_asset";
        if (!filePath.startsWith("/") || filePath.startsWith(AssetsPathPrefix))
        {
            if (filePath.startsWith(AssetsPathPrefix))
            {
                filePath = filePath.substring(AssetsPathPrefix.length() + 1);
            }

            //转换路径为Lua文件目录路径
            filePath = String.format("%s/lua/%s",  getCacheDir(), filePath);
        }

        File f = new File(filePath);
        retValue = LuaNativeUtil.evalScriptFromFile(_nativeId, filePath);

        if (retValue == null)
        {
            retValue = new LuaValue();
        }

        return retValue;
    }

    /**
     * 调用Lua的方法
     * @param methodName    方法名称
     * @param arguments     参数列表
     * @return  返回值
     */
    public LuaValue callMethod(String methodName, LuaValue[] arguments)
    {
        return LuaNativeUtil.callMethod(_nativeId, methodName, arguments);
    }

    /**
     * 注册方法
     * @param methodName    方法名称
     * @param handler       方法处理器
     */
    public void registerMethod(String methodName, LuaMethodHandler handler)
    {
        if (!_methods.containsKey(methodName))
        {
            _methods.put(methodName, handler);
            LuaNativeUtil.registerMethod(_nativeId, methodName);
        }
        else
        {
            throw new Error("Method for the name already exists");
        }
    }

    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private LuaValue methodInvoke (String methodName, LuaValue[] arguments)
    {
        if (_methods.containsKey(methodName))
        {
             return _methods.get(methodName).onExecute(arguments);
        }

        return new LuaValue();
    }

    /**
     * 读取资源文件内容
     * @param fileName  文件名称
     * @param outputStream  输出内容的二进制流
     */
    private void readAssetFileContent(String fileName, ByteArrayOutputStream outputStream)
    {
        try
        {
            InputStream stream = _context.getAssets().open(fileName);
            byte[] buffer = new byte[1024];

            int hasRead = 0;
            while ((hasRead = stream.read(buffer)) != -1)
            {
                outputStream.write(buffer, 0, hasRead);
            }

            stream.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 写入文件
     * @param file  目标文件
     * @param dataStream    二进制流
     */
    private void writeToFile(File file, ByteArrayOutputStream dataStream)
    {
        try
        {
            FileOutputStream stream = new FileOutputStream(file);
            dataStream.writeTo(stream);
            stream.flush();
            stream.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}
