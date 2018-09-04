package cn.vimfung.luascriptcore.modules.foundation;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Environment;

import java.io.File;

import cn.vimfung.luascriptcore.LuaEnv;
import cn.vimfung.luascriptcore.LuaExportType;

import static android.os.Environment.MEDIA_MOUNTED;

/**
 * 路径信息
 */
public final class Path implements LuaExportType
{
    /**
     * 获取应用所在目录
     * @return 路径信息
     */
    public static String appPath()
    {
        Context applicationContext = LuaEnv.defaultEnv().getAndroidApplicationContext();
        return applicationContext.getPackageResourcePath();
    }

    /**
     * 获取应用根目录
     * @return 路径信息
     */
    public static String homePath()
    {
        Context applicationContext = LuaEnv.defaultEnv().getAndroidApplicationContext();

        File homeDir = null;

        int perm = applicationContext.checkCallingOrSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if (MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) &&  perm == PackageManager.PERMISSION_GRANTED)
        {
            homeDir = applicationContext.getExternalFilesDir(null);
        }

        if (homeDir == null)
        {
            homeDir = applicationContext.getFilesDir();
        }

        return homeDir.getAbsolutePath();
    }

    /**
     * 获取应用文档目录
     * @return 路径信息
     */
    public static String docsPath()
    {
        Context applicationContext = LuaEnv.defaultEnv().getAndroidApplicationContext();

        File docDir = null;

        int perm = applicationContext.checkCallingOrSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if (MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) &&  perm == PackageManager.PERMISSION_GRANTED)
        {
            docDir = applicationContext.getExternalFilesDir("Documents");
        }

        if (docDir == null)
        {

            docDir = new File(applicationContext.getFilesDir(), "Documents");
        }

        return docDir.getAbsolutePath();
    }

    /**
     * 获取应用缓存目录
     * @return 路径信息
     */
    public static String cachesPath()
    {
        Context applicationContext = LuaEnv.defaultEnv().getAndroidApplicationContext();

        File appCacheDir = null;

        int perm = applicationContext.checkCallingOrSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if (MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) &&  perm == PackageManager.PERMISSION_GRANTED)
        {
            appCacheDir = applicationContext.getExternalCacheDir();
        }

        if (appCacheDir == null)
        {

            appCacheDir = applicationContext.getCacheDir();
        }

        return appCacheDir.getAbsolutePath();
    }

    /**
     * 获取临时目录信息
     * @return 路径信息
     */
    public static String tmpPath()
    {
        Context applicationContext = LuaEnv.defaultEnv().getAndroidApplicationContext();

        File docDir = null;

        int perm = applicationContext.checkCallingOrSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if (MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) &&  perm == PackageManager.PERMISSION_GRANTED)
        {
            docDir = applicationContext.getExternalFilesDir("tmp");
        }

        if (docDir == null)
        {

            docDir = new File(applicationContext.getFilesDir(), "tmp");
        }

        return docDir.getAbsolutePath();
    }
}
