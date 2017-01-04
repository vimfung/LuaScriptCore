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
	public delegate IntPtr LuaModuleMethodHandleDelegate (int nativeModuleId, string methodName, IntPtr arguments, int size);

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
        /// 创建Lua上下文对象
        /// </summary>
        /// <returns>Lua上下文对象的本地标识</returns>
        [DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int createLuaContext();

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
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int evalScript(int contextId, string script, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册模块
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="moduleName">模块名称</param>
		/// <param name="exportsMethodNames">导出方法名称列表.</param>
		/// <param name="methodRouteHandler">方法路由处理器，所有的方法都会由此方法来回调</param>
		/// <returns>模块的本地标识</returns>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static int registerModule (int nativeContextId, string moduleName, IntPtr exportsMethodNames, IntPtr methodRouteHandler);

		/// <summary>
		/// 判断模块是否注册
		/// </summary>
		/// <returns>true表示注册，false表示尚未注册</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="moduleName">模块名称</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		internal extern static bool isModuleRegisted (int nativeContextId, string moduleName);

		/// <summary>
		/// 设置Unity调试日志接口，用于Lua中输出日志到Unity的编辑器控制台, Editor特有。
		/// </summary>
		/// <param name="fp">方法回调</param>
		[DllImport("LuaScriptCore-Unity-OSX")]
		private extern static void setUnityDebugLog (IntPtr fp);

#elif UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN

        /// <summary>
        /// 创建Lua上下文对象
        /// </summary>
        /// <returns>Lua上下文对象的本地标识</returns>
        [DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int createLuaContext();

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
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int evalScript(int contextId, string script, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 设置Unity调试日志接口，用于Lua中输出日志到Unity的编辑器控制台, Editor特有。
		/// </summary>
		/// <param name="fp">方法回调</param>
		[DllImport("LuaScriptCore-Unity-Win64")]
		private extern static void setUnityDebugLog (IntPtr fp);

#elif UNITY_IPHONE

        /// <summary>
        /// 创建Lua上下文对象
        /// </summary>
        /// <returns>Lua上下文对象的本地标识</returns>
        [DllImport("__Internal")]
		internal extern static int createLuaContext();

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
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("__Internal")]
		internal extern static int evalScript(int contextId, string script, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("__Internal")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("__Internal")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("__Internal")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("__Internal")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册模块
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="moduleName">模块名称</param>
		/// <param name="exportsMethodNames">导出方法名称列表.</param>
		/// <param name="methodRouteHandler">方法路由处理器，所有的方法都会由此方法来回调</param>
		/// <returns>模块的本地标识</returns>
		[DllImport("__Internal")]
		internal extern static int registerModule (int nativeContextId, string moduleName, IntPtr exportsMethodNames, IntPtr methodRouteHandler);

		/// <summary>
		/// 判断模块是否注册
		/// </summary>
		/// <returns>true表示注册，false表示尚未注册</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="moduleName">模块名称</param>
		[DllImport("__Internal")]
		internal extern static bool isModuleRegisted (int nativeContextId, string moduleName);

#elif UNITY_ANDROID

		/// <summary>
		/// 创建Lua上下文对象
		/// </summary>
		/// <returns>Lua上下文对象的本地标识</returns>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int createLuaContext();

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
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="contextId">Lua上下文对象的本地标识</param>
		/// <param name="script">Lua脚本</param>
		/// <param name="resultBuffer">返回值缓冲区</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int evalScript(int contextId, string script, out IntPtr resultBuffer);

		/// <summary>
		/// 从Lua文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int evalScriptFromFile(int nativeContextId, string filePath, out IntPtr resultBuffer);

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值的缓冲区大小</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="resultBuffer">返回值缓冲区.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int callMethod(int nativeContextId, string methodName, IntPtr arguments, out IntPtr resultBuffer);

		/// <summary>
		/// 注册lua方法
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="methodHandler">方法处理器</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void registerMethod (int nativeContextId, string methodName, IntPtr methodHandler);

		/// <summary>
		/// 释放本地对象
		/// </summary>
		/// <param name="objectId">本地对象标识.</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static void releaseObject(int objectId);

		/// <summary>
		/// 注册模块
		/// </summary>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="moduleName">模块名称</param>
		/// <param name="exportsMethodNames">导出方法名称列表.</param>
		/// <param name="methodRouteHandler">方法路由处理器，所有的方法都会由此方法来回调</param>
		/// <returns>模块的本地标识</returns>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static int registerModule (int nativeContextId, string moduleName, IntPtr exportsMethodNames, IntPtr methodRouteHandler);

		/// <summary>
		/// 判断模块是否注册
		/// </summary>
		/// <returns>true表示注册，false表示尚未注册</returns>
		/// <param name="nativeContextId">Lua上下文对象的本地标识</param>
		/// <param name="moduleName">模块名称</param>
		[DllImport("LuaScriptCore-Unity-Android")]
		internal extern static bool isModuleRegisted (int nativeContextId, string moduleName);

#endif
	}
}