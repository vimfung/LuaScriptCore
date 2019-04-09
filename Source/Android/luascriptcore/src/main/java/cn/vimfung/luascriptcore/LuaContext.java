package cn.vimfung.luascriptcore;

import android.app.Application;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Path;
import android.os.AsyncTask;
import android.os.Debug;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.provider.ContactsContract;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.Console;
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
import java.util.concurrent.Callable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import dalvik.system.DexClassLoader;
import dalvik.system.DexFile;
import dalvik.system.PathClassLoader;

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
    private ArrayList<Class<? extends LuaExportType>> _regTypes = null;

    /**
     * 建立Lua目录标识
     */
    private static boolean _isSetupLuaFolder = false;

    /**
     * 获取应用上下文对象
     * @return 上下文对象
     */
    Context getApplicationContext()
    {
        return _context.getApplicationContext();
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
//                        Log.v("lsc", String.format("======copy file = %s", filePath));
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
     * 创建上下文对象
     * @param nativeId  本地对象标识
     */
    protected LuaContext(int nativeId)
    {
        super(nativeId);

        this._regTypes = new ArrayList<>();
        this._methods = new HashMap<>();
    }

    /**
     * 创建上下文对象
     * @param env  环境信息
     * @return  Lua上下文对象
     */
    public static LuaContext create(LuaEnv env)
    {
        LuaContext luaContext = LuaNativeUtil.createContext();
        luaContext._context = env.getAndroidApplicationContext();

        File cacheDir = new File (String.format("%s/lua", luaContext.getCacheDir()));
        if (!cacheDir.exists())
        {
            cacheDir.mkdirs();
        }
        luaContext.addSearchPath(cacheDir.toString());

        return luaContext;
    }

    /**
     * 创建上下文对象
     *
     * @param context  应用上下文对象
     * @return Lua上下文对象
     */
    public static LuaContext create(Context context)
    {
        LuaEnv env = LuaEnv.defaultEnv();
        env.setAndroidApplicationContext(context);
        return LuaContext.create(env);
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

    public void raiseException (String message) throws Error
    {
        LuaNativeUtil.raiseException(this, message);
    }

    /**
     * 添加搜索路径, 对于需要引用不同目录下的lua文件,需要设置其搜索路径,否则会导致无法找到脚本而运行出错
     * @param path  搜索路径
     */
    public void addSearchPath (String path)
    {
        String regExp = "/([^/]+)[.]([^/]+)$";
        if (!Pattern.matches(regExp, path))
        {
            if (!path.endsWith("/"))
            {
                path = path + "/";
            }

            path = path + "?.lua";
        }

        LuaNativeUtil.addSearchPath(_nativeId, path);
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
       return evalScript(script, null);
    }

    /**
     * 解析Lua脚本
     * @param script    脚本
     * @param scriptController  脚本控制器
     * @return 执行后返回值
     */
    public LuaValue evalScript (String script, LuaScriptController scriptController)
    {
        //拷贝资源包中的所有lua文件到临时目录中
        //fixed : lua字符串包含对资源包中文件的引用
        setupLuaFolder();

        return LuaNativeUtil.evalScript(_nativeId, script, scriptController);
    }

    /**
     * 解析Lua脚本文件
     * @param filePath  Lua文件路径
     * @return  执行后返回值
     */
    public LuaValue evalScriptFromFile (String filePath)
    {
        return evalScriptFromFile(filePath, null);
    }

    /**
     * 解析Lua脚本文件
     * @param filePath Lua文件路径
     * @param scriptController  脚本控制器
     * @return 执行后返回值
     */
    public LuaValue evalScriptFromFile (String filePath, LuaScriptController scriptController)
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
        retValue = LuaNativeUtil.evalScriptFromFile(_nativeId, filePath, scriptController);

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
        return callMethod(methodName, arguments, null);
    }

    /**
     * 调用Lua的方法
     * @param methodName    方法名称
     * @param arguments     参数列表
     * @param scriptController  脚本控制器
     * @return  返回值
     */
    public LuaValue callMethod(String methodName, LuaValue[] arguments, LuaScriptController scriptController)
    {
        return LuaNativeUtil.callMethod(_nativeId, methodName, arguments, scriptController);
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
     * 执行线程
     * @param handler 线程处理器
     * @param arguments 参数列表
     */
    public void runThread(LuaFunction handler, LuaValue[] arguments)
    {
        runThread(handler, arguments, null);
    }

    /**
     * 执行线程
     * @param handler 线程处理器
     * @param arguments 参数列表
     * @param scriptController 脚本控制器
     */
    public void runThread(LuaFunction handler, LuaValue[] arguments, LuaScriptController scriptController)
    {
        LuaNativeUtil.runThread(this, handler, arguments, scriptController);
    }

    /**
     * 调用方法
     * @param methodName    方法名称
     * @param arguments     方法的传入参数
     * @return              返回值
     */
    private LuaValue methodInvoke (String methodName, LuaValue[] arguments)
    {

        LuaValue retValue = null;
        if (_methods.containsKey(methodName))
        {
            retValue = _methods.get(methodName).onExecute(arguments);
        }
        else
        {
            retValue = new LuaValue();
        }

        return retValue;

    }

    /**
     * 导出原生类型
     * @param typeName 类型名称
     */
    @SuppressWarnings("unchecked")
    private void exportsNativeType(String typeName)
    {
        //查找类型
        if (typeName.startsWith("_"))
        {
            //无效类型
            return;
        }

        try
        {
            String targetName = typeName.replace("_", ".");
            Class type = Class.forName(targetName);
            exportsNativeType(type);
        }
        catch (ClassNotFoundException e1)
        {
            List<PackageInfo> packageInfos = _context.getPackageManager().getInstalledPackages(0);
            for (PackageInfo pi: packageInfos)
            {
                try
                {
                    Class type = Class.forName(String.format("%s.%s", pi.packageName, typeName));
                    exportsNativeType(type);
                    break;
                }
                catch (ClassNotFoundException e2)
                {

                }
            }
        }

    }

    /**
     * 导出原生类型
     * @param type  类型
     */
    @SuppressWarnings("unchecked")
    private void exportsNativeType(Class type)
    {
        if (!type.isInterface() && LuaExportType.class.isAssignableFrom(type) && !_regTypes.contains(type))
        {
            Class baseType = type.getSuperclass();
            exportsNativeType(baseType);

            Log.d("luascriptcore", String.format("Register type %s", type.getName()));

            _regTypes.add(type);
            LuaExportTypeManager.getDefaultManager().exportType(this, type, baseType);
        }
    }


    /**
     * 读取资源文件内容
     * @param fileName  文件名称
     * @param outputStream  输出内容的二进制流
     */
    private void readAssetFileContent(String fileName, ByteArrayOutputStream outputStream)
    {
        InputStream stream = null;
        try
        {
            stream = _context.getAssets().open(fileName);
            byte[] buffer = new byte[1024];

            int hasRead = 0;
            while ((hasRead = stream.read(buffer)) != -1)
            {
                outputStream.write(buffer, 0, hasRead);
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        finally
        {
            if (stream != null)
            {
                try
                {
                    stream.close();
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }
            }
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
