package cn.vimfung.luascriptcore;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Set;

/**
 * 本地接口工具类
 * Created by vimfung on 16/8/29.
 */
public class LuaNativeUtil
{
    static
    {
        System.loadLibrary("LuaScriptCore");
    }

    private LuaNativeUtil()
    {
        throw new Error("Not allowed to instantiate the class");
    }

    /**
     * 创建Lua上下文对象
     *
     * @return Lua上下文对象
     */
    public static native LuaContext createContext ();

    /**
     * 添加搜索路径, 对于需要引用不同目录下的lua文件,需要设置其搜索路径,否则会导致无法找到脚本而运行出错
     * @param contextNativeId 上下文的本地标识
     * @param path  路径
     */
    public static native void addSearchPath(int contextNativeId, String path);

    /**
     * 设置全局变量
     * @param contextNativeId 上下文的本地标识
     * @param name  变量名称
     * @param value 变量值
     */
    public static native void setGlobal(int contextNativeId, String name, LuaValue value);

    /**
     * 获取全局变量
     * @param contextNativeId  上下文的本地标识
     * @param name  变量名称
     * @return 变量值
     */
    public static native LuaValue getGlobal(int contextNativeId, String name);

    /**
     * 保留LuaValue的Lua对象
     * @param context   上下文对象
     * @param value  值对象
     */
    public static native void retainValue(LuaContext context, LuaValue value);

    /**
     * 释放LuaValue的Lua对象
     * @param context   上下文对象
     * @param value     值对象
     */
    public static native void releaseValue(LuaContext context, LuaValue value);

    /**
     * 排除异常
     * @param context   上下文对象
     * @param message   异常消息
     */
    public static native void raiseException(LuaContext context, String message);

    /**
     * 捕获Lua异常
     * @param context   上下文对象
     * @param enabled   true表示捕获异常，false表示不进行捕获
     */
    public static native void catchException(LuaContext context, boolean enabled);

    /**
     * 解析Lua脚本
     * @param contextNativeId   上下文的本地标识
     * @param script            Lua脚本
     * @param scriptController  脚本控制器
     * @return                  返回值
     */
    public static native LuaValue evalScript (int contextNativeId, String script, LuaScriptController scriptController);

    /**
     * 解析Lua脚本文件
     * @param contextNativeId   上下文的本地标识
     * @param path              Lua脚本文件路径
     * @return                  返回值
     */
    public static native LuaValue evalScriptFromFile (int contextNativeId, String path, LuaScriptController scriptController);

    /**
     * 执行Lua方法
     * @param contextNativeId   上下文的本地标识
     * @param methodName        方法名称
     * @param arguments         方法参数列表
     * @return                  返回值
     */
    public static native LuaValue callMethod (int contextNativeId, String methodName, LuaValue[] arguments, LuaScriptController scriptController);

    /**
     * 注册Lua方法
     * @param contextNativeId   上下文的本地标识
     * @param methodName        方法名称
     */
    public static native void registerMethod (int contextNativeId, String methodName);

    /**
     * 释放本地对象
     * @param nativeId  本地对象标识
     */
    public static native void releaseNativeObject (int nativeId);

    /**
     * 调用方法
     * @param context   上下文对象
     * @param func      方法对象
     * @param arguments 参数列表
     * @param scriptController 脚本控制器
     * @return  返回值
     */
    public static native LuaValue invokeFunction (
            LuaContext context,
            LuaFunction func,
            LuaValue[] arguments,
            LuaScriptController scriptController);

    /**
     * 注册类型
     * @param context               上下文对象
     * @param alias                 别名
     * @param typeName              导出类型名称
     * @param parentTypeName        导出类型的父类名称
     * @param type                  导出类型
     * @param fields                导出字段集合
     * @param instanceMethods       导出实例方法集合
     * @param classMethods          导出类方法集合
     * @return  true 注册成功， false 注册失败
     */
    public static native boolean registerType (
            LuaContext context,
            String alias,
            String typeName,
            String parentTypeName,
            Class type,
            String[] fields,
            String[] instanceMethods,
            String[] classMethods);

    /**
     * 运行线程
     * @param context 上下文对象
     * @param handler 事件处理器
     * @param arguments 参数列表
     * @param scriptController 脚本控制器
     */
    public static native void runThread(LuaContext context,
                                        LuaFunction handler,
                                        LuaValue[] arguments,
                                        LuaScriptController scriptController);

    /**
     * 创建脚本控制器
     *
     * @return 脚本控制器
     */
    public static native LuaScriptController createScriptController ();

    /**
     * 设置脚本控制器超时时间
     * @param controller 脚本控制器
     * @param timeout 超时时间
     */
    public static native void scriptControllerSetTimeout(LuaScriptController controller, int timeout);

    /**
     * 强制退出脚本执行
     * @param controller 脚本控制器
     */
    public static native void scriptControllerForceExit(LuaScriptController controller);

    /**
     * LuaValue设置对应键名指向对象，仅用于ValueType为Map
     * @param context 上下文对象
     * @param value 值对象
     * @param keyPath 键名路径
     * @param object 对象
     * @return 返回更新后对象
     */
    public static native Object luaValueSetObject(LuaContext context, LuaValue value, String keyPath, LuaValue object);

}
