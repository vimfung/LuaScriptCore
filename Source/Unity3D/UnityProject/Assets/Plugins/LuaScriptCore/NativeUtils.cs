using System.Collections;
using System.Runtime.InteropServices;
using System;
using UnityEngine;
using AOT;

namespace cn.vimfung.luascriptcore
{
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate IntPtr LuaMethodHandleDelegate(int nativeContextId, string methodName, IntPtr arguments, int size);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LuaSetNativeObjectIdHandleDelegate(Int64 obj, int nativeObjectId, string luaObjectId);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate string LuaGetClassNameByInstanceDelegate(IntPtr obj);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LuaExportsNativeTypeDelegate(int contextId, string typeName);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate Int64 LuaInstanceCreateHandleDelegate(int contextId, int nativeClassId, IntPtr argumentsBuffer, int bufferSize);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LuaInstanceDestroyHandleDelegate(Int64 instancePtr);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate string LuaInstanceDescriptionHandleDelegate(Int64 instancePtr);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate IntPtr LuaModuleMethodHandleDelegate (int contextId, int nativeModuleId, string methodName, IntPtr arguments, int size);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate IntPtr LuaInstanceMethodHandleDelegate (int contextId, int classId, Int64 instance, string methodName, IntPtr argumentsBuffer, int bufferSize);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate IntPtr LuaInstanceFieldGetterHandleDelegate (int contextId, int classId, Int64 instance, string fieldName);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LuaInstanceFieldSetterHandleDelegate (int contextId,int classId, Int64 instance, string fieldName, IntPtr valueBuffer, int bufferSize);

	/// <summary>
	/// 异常处理器
	/// </summary>
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LuaExceptionHandleDelegate(int contextId, string errMessage);

	public class NativeUtils  
	{
#if UNITY_EDITOR

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void UnityDegubLogDelegate(string str);

		static UnityDegubLogDelegate _debugLogDelegate = null;

		static NativeUtils ()
		{
			//替换Lua中的日志输出
			_debugLogDelegate = new UnityDegubLogDelegate(UnityDebugLogHandler);
			IntPtr fp = Marshal.GetFunctionPointerForDelegate(_debugLogDelegate);
			setUnityDebugLog (fp);
		}

		/// <summary>
		/// 调试日志处理
		/// </summary>
		/// <param name="str">日志内容</param> 
		[MonoPInvokeCallback (typeof (UnityDegubLogDelegate))]
		static void UnityDebugLogHandler(string str)
		{
			if (str == "\n") 
			{
				return;
			}

			Debug.Log(str);
		}

#endif


#if UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX

		/// <summary>
		/// 绑定设置原生对象ID处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void bindSetNativeObjectIdHandler(IntPtr handler);

		/// <summary>
		/// 绑定根据实例获取类型名称处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void bindGetClassNameByInstanceHandler (IntPtr handler);

		/// <summary>
		/// 绑定导出原生类型处理器
		/// </summary>
		/// <param name="handler">处理器.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void bindExportsNativeTypeHandler(IntPtr handler);

        /// <summary>
        /// 创建Lua上下文对象
        /// </summary>
        /// <returns>Lua上下文对象的本地标识</returns>
        [DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int createLuaContext();

		/// <summary>
		/// 创建脚本控制器
		/// </summary>
		/// <returns>脚本控制器对象标识.</returns>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int createLuaScriptController ();

		/// <summary>
		/// 设置脚本控制器超时
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		/// <param name="timeout">超时时间，单位：秒，如果传入0则为不限制超时.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void scriptControllerSetTimeout (int scriptControllerId, int timeout);

		/// <summary>
		/// 强制退出脚本执行
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void scriptControllerForceExit (int scriptControllerId);

		/// <summary>
		/// 添加Lua的搜索路径，如果在采用require方法时无法导入其他路径脚本，可能是由于脚本文件的所在路径不在lua的搜索路径中。
		/// 可通过此方法进行添加。
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="path">路径.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void addSearchPath (int contextId, string path);

		/// <summary>
		/// 设置异常处理器
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="HandleRef">事件处理器引用.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void setExceptionHandler (int contextId, IntPtr handlerRef);

		/// <summary>
		/// 抛出异常
		/// </summary>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="message">消息.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void raiseException (int contextId, string message);

		/// <summary>
		/// 设置全局变量
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="value">变量值.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void setGlobal (int contextId, string name, IntPtr value);

		/// <summary>
		/// 获取全局变量
		/// </summary>
		/// <returns>全局变量值缓存长度.</returns>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="resultBuffer">返回值缓存.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int getGlobal (int contextId, string name, out IntPtr resultBuffer);

		/// <summary>
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int evalScript(int contextId, string script, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识.</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表.</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int invokeLuaFunction (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);  

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册类型
		/// </summary>
		/// <returns>The type.</returns>
		/// <param name="nativeContextId">本地上下文标识.</param>
		/// <param name="alias">别名</param>
		/// <param name="typeName">类型名称.</param>
		/// <param name="parentTypeName">父类名称.</param>
		/// <param name="exportsPropertyNames">导出属性名称列表</param>
		/// <param name="exportsInstanceMethodNames">导出实例方法名称列表.</param>
		/// <param name="xportsClassMethodNames">导出类方法名称列表.</param>
		/// <param name="instanceCreateHandler">创建实例处理器.</param>
		/// <param name="instanceDestroyHandler">销毁实例处理器.</param>
		/// <param name="instanceDescriptionHandler">实例描述处理器.</param>
		/// <param name="instanceFieldGetterRouteHandler">实例字段Getter处理器.</param>
		/// <param name="instanceFieldSetterRouteHandler">实例字段Setter处理器.</param>
		/// <param name="instanceMethodRouteHandler">实例方法处理器.</param>
		/// <param name="classMethodRouteHandler">类方法处理器.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int registerType(
			int nativeContextId,
			string alias,
			string typeName,
			string parentTypeName,
			IntPtr exportsPropertyNames,
			IntPtr exportsInstanceMethodNames,
			IntPtr exportsClassMethodNames,
			IntPtr instanceCreateHandler,
			IntPtr instanceDestroyHandler,
			IntPtr instanceDescriptionHandler,
			IntPtr instanceFieldGetterRouteHandler,
			IntPtr instanceFieldSetterRouteHandler,
			IntPtr instanceMethodRouteHandler,
			IntPtr classMethodRouteHandler);

		/// <summary>
		/// 保留LuaValue的对象
		/// </summary>
		/// <param name="nativeContextId">原生上下文对象标识.</param>
		/// <param name="value">值对象.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void retainValue (int nativeContextId, IntPtr value);

		/// <summary>
		/// 释放LuaValue的对象
		/// </summary>
		/// <param name="nativeContextId">原生上下文对象标识.</param>
		/// <param name="value">值对象.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void releaseValue (int nativeContextId, IntPtr value);

		/// <summary>
		/// 执行线程
		/// </summary>
		/// <param name="nativeContextId">上下文标识</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void runThread (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId);

		/// <summary>
		/// 设置指定table的键值
		/// </summary>
		/// <returns>缓存大小</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="value">值对象</param>
		/// <param name="keyPath">键名路径.</param>
		/// <param name="obj">设置值.</param>
		/// <param name="resultBuffer">返回缓存，用于同步LuaValue中的Map对象.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int tableSetObject (int contextId, IntPtr value, string keyPath, IntPtr obj, IntPtr resultBuffer);

		/// <summary>
		/// 设置Unity调试日志接口，用于Lua中输出日志到Unity的编辑器控制台, Editor特有。
		/// </summary>
		/// <param name="fp">方法回调</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		private extern static void setUnityDebugLog (IntPtr fp);


#elif UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN

        /// <summary>
		/// 绑定设置原生对象ID处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
        internal extern static void bindSetNativeObjectIdHandler(IntPtr handler);

        /// <summary>
        /// 绑定根据实例获取类型名称处理器
        /// </summary>
        /// <param name="handler">处理器对象</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
        internal extern static void bindGetClassNameByInstanceHandler(IntPtr handler);

		/// <summary>
		/// 绑定导出原生类型处理器
		/// </summary>
		/// <param name="handler">处理器.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void bindExportsNativeTypeHandler(IntPtr handler);


        /// <summary>
        /// 创建Lua上下文对象
        /// </summary>
        /// <returns>Lua上下文对象的本地标识</returns>
        [DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int createLuaContext();

		/// <summary>
		/// 创建脚本控制器
		/// </summary>
		/// <returns>脚本控制器对象标识.</returns>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int createLuaScriptController ();

		/// <summary>
		/// 设置脚本控制器超时
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		/// <param name="timeout">超时时间，单位：秒，如果传入0则为不限制超时.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void scriptControllerSetTimeout (int scriptControllerId, int timeout);

		/// <summary>
		/// 强制退出脚本执行
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void scriptControllerForceExit (int scriptControllerId);

		/// <summary>
		/// 添加Lua的搜索路径，如果在采用require方法时无法导入其他路径脚本，可能是由于脚本文件的所在路径不在lua的搜索路径中。
		/// 可通过此方法进行添加。
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="path">路径.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void addSearchPath (int contextId, string path);

		/// <summary>
		/// 设置异常处理器
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="HandleRef">事件处理器引用.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void setExceptionHandler (int contextId, IntPtr handlerRef);

		/// <summary>
		/// 抛出异常
		/// </summary>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="message">消息.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void raiseException (int contextId, string message);

        /// <summary>
		/// 设置全局变量
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="value">变量值.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
        internal extern static void setGlobal(int contextId, string name, IntPtr value);

        /// <summary>
        /// 获取全局变量
        /// </summary>
        /// <returns>全局变量值缓存长度.</returns>
        /// <param name="contextId">Lua上下文对象的本地标识.</param>
        /// <param name="name">变量名称.</param>
        /// <param name="resultBuffer">返回值缓存.</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
        internal extern static int getGlobal(int contextId, string name, out IntPtr resultBuffer);

        /// <summary>
        /// 解析Lua脚本
        /// </summary>
        /// <returns>返回值的缓冲区大小</returns>
        /// <param name="contextId">Lua上下文对象的本地标识</param>
        /// <param name="script">Lua脚本</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
        /// <param name="resultBuffer">返回值缓冲区</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int evalScript(int contextId, string script, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

        /// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识.</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表.</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int invokeLuaFunction(int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

        /// <summary>
        /// 释放本地对象
        /// </summary>
        /// <param name="objectId">本地对象标识.</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册类型
		/// </summary>
		/// <returns>The type.</returns>
		/// <param name="nativeContextId">本地上下文标识.</param>
		/// <param name="alias">别名</param>
		/// <param name="typeName">类型名称.</param>
		/// <param name="parentTypeName">父类名称.</param>
		/// <param name="exportsPropertyNames">导出属性名称列表</param>
		/// <param name="exportsInstanceMethodNames">导出实例方法名称列表.</param>
		/// <param name="xportsClassMethodNames">导出类方法名称列表.</param>
		/// <param name="instanceCreateHandler">创建实例处理器.</param>
		/// <param name="instanceDestroyHandler">销毁实例处理器.</param>
		/// <param name="instanceDescriptionHandler">实例描述处理器.</param>
		/// <param name="instanceFieldGetterRouteHandler">实例字段Getter处理器.</param>
		/// <param name="instanceFieldSetterRouteHandler">实例字段Setter处理器.</param>
		/// <param name="instanceMethodRouteHandler">实例方法处理器.</param>
		/// <param name="classMethodRouteHandler">类方法处理器.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int registerType(
			int nativeContextId,
			string alias,
			string typeName,
			string parentTypeName,
			IntPtr exportsPropertyNames,
			IntPtr exportsInstanceMethodNames,
			IntPtr exportsClassMethodNames,
			IntPtr instanceCreateHandler,
			IntPtr instanceDestroyHandler,
			IntPtr instanceDescriptionHandler,
			IntPtr instanceFieldGetterRouteHandler,
			IntPtr instanceFieldSetterRouteHandler,
			IntPtr instanceMethodRouteHandler,
			IntPtr classMethodRouteHandler);

        /// <summary>
        /// 保留LuaValue的对象
        /// </summary>
        /// <param name="nativeContextId">原生上下文对象标识.</param>
        /// <param name="value">值对象.</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
        internal extern static void retainValue(int nativeContextId, IntPtr value);

        /// <summary>
        /// 释放LuaValue的对象
        /// </summary>
        /// <param name="nativeContextId">原生上下文对象标识.</param>
        /// <param name="value">值对象.</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
        internal extern static void releaseValue(int nativeContextId, IntPtr value);

		/// <summary>
		/// 执行线程
		/// </summary>
		/// <param name="nativeContextId">上下文标识</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void runThread (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId);

		/// <summary>
		/// 设置指定table的键值
		/// </summary>
		/// <returns>缓存大小</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="value">值对象</param>
		/// <param name="keyPath">键名路径.</param>
		/// <param name="obj">设置值.</param>
		/// <param name="resultBuffer">返回缓存，用于同步LuaValue中的Map对象.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int tableSetObject (int contextId, IntPtr value, string keyPath, IntPtr obj, IntPtr resultBuffer);

        /// <summary>
        /// 设置Unity调试日志接口，用于Lua中输出日志到Unity的编辑器控制台, Editor特有。
        /// </summary>
        /// <param name="fp">方法回调</param>
        [DllImport("LuaScriptCore-Unity-Win64")]
		private extern static void setUnityDebugLog (IntPtr fp);

#elif UNITY_IPHONE

		/// <summary>
		/// 绑定设置原生对象ID处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("__Internal")]
		internal extern static void bindSetNativeObjectIdHandler(IntPtr handler);

		/// <summary>
		/// 绑定根据实例获取类型名称处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("__Internal")]
		internal extern static void bindGetClassNameByInstanceHandler (IntPtr handler);

		/// <summary>
		/// 绑定导出原生类型处理器
		/// </summary>
		/// <param name="handler">处理器.</param>
		[DllImport("__Internal")]
		internal extern static void bindExportsNativeTypeHandler(IntPtr handler);

		/// <summary>
		/// 设置全局变量
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="value">变量值.</param>
		[DllImport("__Internal")]
		internal extern static void setGlobal (int contextId, string name, IntPtr value);

		/// <summary>
		/// 获取全局变量
		/// </summary>
		/// <returns>全局变量值缓存长度.</returns>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="resultBuffer">返回值缓存.</param>
		[DllImport("__Internal")]
		internal extern static int getGlobal (int contextId, string name, out IntPtr resultBuffer);

        /// <summary>
        /// 创建Lua上下文对象
        /// </summary>
        /// <returns>Lua上下文对象的本地标识</returns>
        [DllImport("__Internal")]
		internal extern static int createLuaContext();

		/// <summary>
		/// 创建脚本控制器
		/// </summary>
		/// <returns>脚本控制器对象标识.</returns>
		[DllImport("__Internal")]
		internal extern static int createLuaScriptController ();

		/// <summary>
		/// 设置脚本控制器超时
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		/// <param name="timeout">超时时间，单位：秒，如果传入0则为不限制超时.</param>
		[DllImport("__Internal")]
		internal extern static void scriptControllerSetTimeout (int scriptControllerId, int timeout);

		/// <summary>
		/// 强制退出脚本执行
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		[DllImport("__Internal")]
		internal extern static void scriptControllerForceExit (int scriptControllerId);

		/// <summary>
		/// 添加Lua的搜索路径，如果在采用require方法时无法导入其他路径脚本，可能是由于脚本文件的所在路径不在lua的搜索路径中。
		/// 可通过此方法进行添加。
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="path">路径.</param>
		[DllImport("__Internal")]
		internal extern static void addSearchPath (int contextId, string path);

		/// <summary>
		/// 设置异常处理器
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="HandleRef">事件处理器引用.</param>
		[DllImport("__Internal")]
		internal extern static void setExceptionHandler (int contextId, IntPtr handlerRef);

		/// <summary>
		/// 抛出异常
		/// </summary>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="message">消息.</param>
		[DllImport("__Internal")]
		internal extern static void raiseException (int contextId, string message);

		/// <summary>
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("__Internal")]
		internal extern static int evalScript(int contextId, string script, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("__Internal")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("__Internal")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("__Internal")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识.</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表.</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("__Internal")]
		internal extern static int invokeLuaFunction (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("__Internal")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册类型
		/// </summary>
		/// <returns>The type.</returns>
		/// <param name="nativeContextId">本地上下文标识.</param>
		/// <param name="alias">别名</param>
		/// <param name="typeName">类型名称.</param>
		/// <param name="parentTypeName">父类名称.</param>
		/// <param name="exportsPropertyNames">导出属性名称列表</param>
		/// <param name="exportsInstanceMethodNames">导出实例方法名称列表.</param>
		/// <param name="xportsClassMethodNames">导出类方法名称列表.</param>
		/// <param name="instanceCreateHandler">创建实例处理器.</param>
		/// <param name="instanceDestroyHandler">销毁实例处理器.</param>
		/// <param name="instanceDescriptionHandler">实例描述处理器.</param>
		/// <param name="instanceFieldGetterRouteHandler">实例字段Getter处理器.</param>
		/// <param name="instanceFieldSetterRouteHandler">实例字段Setter处理器.</param>
		/// <param name="instanceMethodRouteHandler">实例方法处理器.</param>
		/// <param name="classMethodRouteHandler">类方法处理器.</param>
		[DllImport("__Internal")]
		internal extern static int registerType(
			int nativeContextId,
			string alias,
			string typeName,
			string parentTypeName,
			IntPtr exportsPropertyNames,
			IntPtr exportsInstanceMethodNames,
			IntPtr exportsClassMethodNames,
			IntPtr instanceCreateHandler,
			IntPtr instanceDestroyHandler,
			IntPtr instanceDescriptionHandler,
			IntPtr instanceFieldGetterRouteHandler,
			IntPtr instanceFieldSetterRouteHandler,
			IntPtr instanceMethodRouteHandler,
			IntPtr classMethodRouteHandler);

		/// <summary>
		/// 保留LuaValue的对象
		/// </summary>
		/// <param name="nativeContextId">原生上下文对象标识.</param>
		/// <param name="value">值对象.</param>
		[DllImport("__Internal")]
		internal extern static void retainValue (int nativeContextId, IntPtr value);

		/// <summary>
		/// 释放LuaValue的对象
		/// </summary>
		/// <param name="nativeContextId">原生上下文对象标识.</param>
		/// <param name="value">值对象.</param>
		[DllImport("__Internal")]
		internal extern static void releaseValue (int nativeContextId, IntPtr value);

		/// <summary>
		/// 执行线程
		/// </summary>
		/// <param name="nativeContextId">上下文标识</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		[DllImport("__Internal")]
		internal extern static void runThread (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId);

		/// <summary>
		/// 设置指定table的键值
		/// </summary>
		/// <returns>缓存大小</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="value">值对象</param>
		/// <param name="keyPath">键名路径.</param>
		/// <param name="obj">设置值.</param>
		/// <param name="resultBuffer">返回缓存，用于同步LuaValue中的Map对象.</param>
		[DllImport("__Internal")]
		internal extern static int tableSetObject (int contextId, IntPtr value, string keyPath, IntPtr obj, IntPtr resultBuffer);

#elif UNITY_ANDROID

		/// <summary>
		/// 绑定设置原生对象ID处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void bindSetNativeObjectIdHandler(IntPtr handler);

		/// <summary>
		/// 绑定根据实例获取类型名称处理器
		/// </summary>
		/// <param name="handler">处理器对象</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void bindGetClassNameByInstanceHandler (IntPtr handler);

		/// <summary>
		/// 绑定导出原生类型处理器
		/// </summary>
		/// <param name="handler">处理器.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void bindExportsNativeTypeHandler(IntPtr handler);

		/// <summary>
		/// 设置全局变量
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="value">变量值.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void setGlobal (int contextId, string name, IntPtr value);

		/// <summary>
		/// 获取全局变量
		/// </summary>
		/// <returns>全局变量值缓存长度.</returns>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="name">变量名称.</param>
		/// <param name="resultBuffer">返回值缓存.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int getGlobal (int contextId, string name, out IntPtr resultBuffer);

		/// <summary>
		/// 创建Lua上下文对象
		/// </summary>
		/// <returns>Lua上下文对象的本地标识</returns>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int createLuaContext();

		/// <summary>
		/// 创建脚本控制器
		/// </summary>
		/// <returns>脚本控制器对象标识.</returns>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int createLuaScriptController ();

		/// <summary>
		/// 设置脚本控制器超时
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		/// <param name="timeout">超时时间，单位：秒，如果传入0则为不限制超时.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void scriptControllerSetTimeout (int scriptControllerId, int timeout);

		/// <summary>
		/// 强制退出脚本执行
		/// </summary>
		/// <param name="scriptControllerId">脚本控制器标识.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void scriptControllerForceExit (int scriptControllerId);

		/// <summary>
		/// 添加Lua的搜索路径，如果在采用require方法时无法导入其他路径脚本，可能是由于脚本文件的所在路径不在lua的搜索路径中。
		/// 可通过此方法进行添加。
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="path">路径.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void addSearchPath (int contextId, string path);

		/// <summary>
		/// 设置异常处理器
		/// </summary>
		/// <param name="contextId">Lua上下文对象的本地标识.</param>
		/// <param name="HandleRef">事件处理器引用.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void setExceptionHandler (int contextId, IntPtr handlerRef);

		/// <summary>
		/// 抛出异常
		/// </summary>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="message">消息.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void raiseException (int contextId, string message);

		/// <summary>
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int evalScript(int contextId, string script, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识.</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表.</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int invokeLuaFunction (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId, out IntPtr resultBuffer);

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册类型
		/// </summary>
		/// <returns>The type.</returns>
		/// <param name="nativeContextId">本地上下文标识.</param>
		/// <param name="alias">别名</param>
		/// <param name="typeName">类型名称.</param>
		/// <param name="parentTypeName">父类名称.</param>
		/// <param name="exportsPropertyNames">导出属性名称列表</param>
		/// <param name="exportsInstanceMethodNames">导出实例方法名称列表.</param>
		/// <param name="xportsClassMethodNames">导出类方法名称列表.</param>
		/// <param name="instanceCreateHandler">创建实例处理器.</param>
		/// <param name="instanceDestroyHandler">销毁实例处理器.</param>
		/// <param name="instanceDescriptionHandler">实例描述处理器.</param>
		/// <param name="instanceFieldGetterRouteHandler">实例字段Getter处理器.</param>
		/// <param name="instanceFieldSetterRouteHandler">实例字段Setter处理器.</param>
		/// <param name="instanceMethodRouteHandler">实例方法处理器.</param>
		/// <param name="classMethodRouteHandler">类方法处理器.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int registerType(
			int nativeContextId,
			string alias,
			string typeName,
			string parentTypeName,
			IntPtr exportsPropertyNames,
			IntPtr exportsInstanceMethodNames,
			IntPtr exportsClassMethodNames,
			IntPtr instanceCreateHandler,
			IntPtr instanceDestroyHandler,
			IntPtr instanceDescriptionHandler,
			IntPtr instanceFieldGetterRouteHandler,
			IntPtr instanceFieldSetterRouteHandler,
			IntPtr instanceMethodRouteHandler,
			IntPtr classMethodRouteHandler);

		/// <summary>
		/// 保留LuaValue的对象
		/// </summary>
		/// <param name="nativeContextId">原生上下文对象标识.</param>
		/// <param name="value">值对象.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void retainValue (int nativeContextId, IntPtr value);

		/// <summary>
		/// 释放LuaValue的对象
		/// </summary>
		/// <param name="nativeContextId">原生上下文对象标识.</param>
		/// <param name="value">值对象.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void releaseValue (int nativeContextId, IntPtr value);

		/// <summary>
		/// 执行线程
		/// </summary>
		/// <param name="nativeContextId">上下文标识</param>
		/// <param name="function">方法.</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptControllerId">脚本控制器标识</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void runThread (int nativeContextId, IntPtr function, IntPtr arguments, int scriptControllerId);

		/// <summary>
		/// 设置指定table的键值
		/// </summary>
		/// <returns>缓存大小</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="value">值对象</param>
		/// <param name="keyPath">键名路径.</param>
		/// <param name="obj">设置值.</param>
		/// <param name="resultBuffer">返回缓存，用于同步LuaValue中的Map对象.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int tableSetObject (int contextId, IntPtr value, string keyPath, IntPtr obj, IntPtr resultBuffer);
#endif
	}
}