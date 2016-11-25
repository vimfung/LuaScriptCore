package cn.vimfung.luascriptcore;

import android.app.Application;
import android.content.Context;
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
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;

/**
 * Lua上下文对象
 * Created by vimfung on 16/8/22.
 */
public class LuaContext extends LuaBaseObject
{
    private Context _context;
    private HashMap<String, LuaMethodHandler> _methods;

    /**
     * 建立Lua目录标识
     */
    private static boolean _isSetupLuaFolder = false;

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
                        String filePath = String.format("%s/lua/%s", _context.getExternalCacheDir(), fileName);
                        File file = new File(filePath);
                        File parentFile = file.getParentFile();
                        if (!parentFile.exists())
                        {
                            parentFile.mkdirs();
                        }

                        readAssetFileContent(fileName, dataStream);
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

        this._methods = new HashMap<String, LuaMethodHandler>();
    }

    /**
     * 创建上下文对象
     * @return  上下文对象
     */
    public static LuaContext create(Context context)
    {
        LuaContext luaContext = LuaNativeUtil.createContext();
        luaContext._context = context;

        File cacheDir = new File (String.format("%s/lua", context.getExternalCacheDir()));
        if (!cacheDir.exists())
        {
            cacheDir.mkdirs();
        }
        luaContext.addSearchPath(cacheDir.toString());

        return luaContext;
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
     * 解析Lua脚本
     * @param script  脚本
     * @return 执行后返回的值
     */
    public LuaValue evalScript (String script)
    {
        return LuaNativeUtil.evalScript(_nativeId, script);
    }

    /**
     * 解析Lua脚本文件
     * @param filePath  Lua文件路径
     * @return  执行后返回值
     */
    public LuaValue evalScriptFromFile (String filePath)
    {
        LuaValue retValue = null;

        String AssetsPathPrefix = "/android_asset";
        if (!filePath.startsWith("/") || filePath.startsWith(AssetsPathPrefix))
        {
            if (filePath.startsWith(AssetsPathPrefix))
            {
                filePath = filePath.substring(AssetsPathPrefix.length() + 1);
            }

            //拷贝资源包中的所有lua文件到临时目录中
            setupLuaFolder();
            //转换路径为Lua文件目录路径
            filePath = String.format("%s/lua/%s",  _context.getExternalCacheDir(), filePath);
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
     * 注册模块
     * @param moduleClass     模块类
     */
    public void registerModule(Class<? extends LuaModule> moduleClass)
    {
        try
        {
            Method regMethod = moduleClass.getMethod("_register", LuaContext.class, moduleClass.getClass());
            regMethod.invoke(moduleClass, this, moduleClass);
        }
        catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * 判断模块是否已经注册
     * @param moduleName    模块名称
     * @return  true 模块已注册, 否则尚未注册
     */
    public boolean isModuleRegisted(String moduleName)
    {
        return LuaNativeUtil.isModuleRegisted(_nativeId, moduleName);
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
