using System.Collections;
using UnityEngine;
using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Collections.Generic;
using AOT;
using System.IO;
using System.Threading;
using System.Text.RegularExpressions;
using System.Linq;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// Lua方法处理器
	/// </summary>
	public delegate LuaValue LuaMethodHandler (List<LuaValue> arguments);

	/// <summary>
	/// 异常处理器
               	/// </summary>
	public delegate void LuaExceptionHandler(string errMessage);

	/// <summary>
	/// Lua上下文对象
	/// </summary>
	public class LuaContext : LuaBaseObject
	{
		/// <summary>
		/// 方法处理器集合
		/// </summary>
		private Dictionary<string, LuaMethodHandler> _methodHandlers;

		/// <summary>
		/// 导出类型管理器
		/// </summary>
		private LuaExportsTypeManager _exportsTypeManager;

		/// <summary>
		/// 注册的类型列表
		/// </summary>
		private HashSet<Type> _regTypes;

		/// <summary>
		/// 方法处理委托
		/// </summary>
		private static LuaMethodHandleDelegate _methodHandleDelegate;

		/// <summary>
		/// The contexts.
		/// </summary>
		private static Dictionary<int, WeakReference> _contexts;

		/// <summary>
		/// 第一次使用LuaContext类时触发
		/// </summary>
		static LuaContext()
		{
			UNIEnv.setup ();
			_contexts = new Dictionary<int, WeakReference> ();
		}

		/// <summary>
		/// 获取上下文对象
		/// </summary>
		/// <returns>上下文对象</returns>
		/// <param name="nativeId">对象标识.</param>
		internal static LuaContext getContext(int nativeId)
		{
			if (_contexts.ContainsKey (nativeId))
			{
				WeakReference wr = _contexts [nativeId];
				return wr.Target as LuaContext;
			}
			return null;
		}

		/// <summary>
		/// 获取导出类型管理器
		/// </summary>
		/// <value>导出类型管理器.</value>
		internal LuaExportsTypeManager exportsTypeManager
		{
			get
			{
				return _exportsTypeManager;
			}
		}

		/// <summary>
		/// 导出原生类型
		/// </summary>
		/// <param name="t">类型.</param>
		internal void exportsNativeType(Type t)
		{
			if (t != null 
				&& t.GetInterface("cn.vimfung.luascriptcore.LuaExportType") != null 
				&& !_regTypes.Contains(t))
			{
				//先导出父类
				Type baseType = t.BaseType;
				exportsNativeType (baseType);

				//导出类型
				_regTypes.Add (t);
				exportsTypeManager.exportType (t, this);
			}
		}

		/// <summary>
		/// 初始化上下文
		/// </summary>
		public LuaContext()
		{
			_regTypes = new HashSet<Type> ();
			_methodHandlers = new Dictionary<string, LuaMethodHandler> ();
			_exportsTypeManager = new LuaExportsTypeManager (this);
			_nativeObjectId = NativeUtils.createLuaContext ();
			_contexts.Add (_nativeObjectId, new WeakReference(this));

			//初始化异常消息捕获
			IntPtr fp = Marshal.GetFunctionPointerForDelegate(new LuaExceptionHandleDelegate(luaException));
			NativeUtils.setExceptionHandler (_nativeObjectId, fp);

			#if UNITY_ANDROID && !UNITY_EDITOR

			AndroidJavaObject luaCacheDir = getLuaCacheDir ();
			if (!luaCacheDir.Call<bool> ("exists", new object[0]))
			{
				luaCacheDir.Call<bool> ("mkdirs", new object[0]);
			}

			addSearchPath (luaCacheDir.Call<string> ("toString", new object[0]));

			#else

			//增加搜索路径
			addSearchPath (Application.streamingAssetsPath);

			#endif
		}

		~LuaContext()
		{
			_contexts.Remove (_nativeObjectId);
		}

		/// <summary>
		/// 添加Lua的搜索路径，如果在采用require方法时无法导入其他路径脚本，可能是由于脚本文件的所在路径不在lua的搜索路径中。
		/// 可通过此方法进行添加。
		/// </summary>
		/// <param name="path">路径.</param>
		public void addSearchPath(string path)
		{
			Regex regext = new Regex ("/([^/]+)[.]([^/]+)$");
			if (!regext.IsMatch (path))
			{
				if (!path.EndsWith ("/"))
				{
					path += "/";
				}	

				path += "?.lua";
			}

			if (!path.StartsWith ("/") || path.StartsWith(Application.streamingAssetsPath))
			{
				#if UNITY_ANDROID && !UNITY_EDITOR

				AndroidJavaObject luaCacheDir = getLuaCacheDir ();
				if (!luaCacheDir.Call<bool> ("exists", new object[0]))
				{
					luaCacheDir.Call<bool> ("mkdirs", new object[0]);
				}
				string cachePath = luaCacheDir.Call<string> ("toString", new object[0]);

				if (path.StartsWith(Application.streamingAssetsPath))
				{
					path = path.Substring(Application.streamingAssetsPath.Length + 1);
				}

				path = string.Format("{0}/{1}", cachePath, path);

				#else


				if (!path.StartsWith("/") && !path.StartsWith(Application.streamingAssetsPath))
				{
					path = string.Format("{0}/{1}", Application.streamingAssetsPath, path);
				}

				#endif
			}


			NativeUtils.addSearchPath (_nativeObjectId, path);
		}

		/// <summary>
		/// 异常处理器
		/// </summary>
		private LuaExceptionHandler _exceptionHandler;

		/// <summary>
		/// 异常时触发
		/// </summary>
		/// <param name="handler">事件处理器</param>
		public void onException(LuaExceptionHandler handler)
		{
			_exceptionHandler = handler;
		}

		/// <summary>
		/// 抛出异常
		/// </summary>
		/// <param name="message">异常消息.</param>
		public void raiseException(string message)
		{
			NativeUtils.raiseException (_nativeObjectId, message);
		}

		/// <summary>
		/// 设置全局变量
		/// </summary>
		/// <param name="name">变量名称.</param>
		/// <param name="value">变量值.</param>
		public void setGlobal(string name, LuaValue value)
		{
			IntPtr valuePtr = IntPtr.Zero;
			if (value != null)
			{
				LuaObjectEncoder encoder = new LuaObjectEncoder (this);
				encoder.writeObject (value);

				byte[] bytes = encoder.bytes;
				valuePtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, valuePtr, bytes.Length);
			}
				
			NativeUtils.setGlobal (_nativeObjectId, name, valuePtr);

			if (valuePtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (valuePtr);
			}
		}

		/// <summary>
		/// 获取全局变量
		/// </summary>
		/// <returns>变量值.</returns>
		/// <param name="name">变量名称.</param>
		public LuaValue getGlobal(string name)
		{
			IntPtr valuePtr = IntPtr.Zero;
			int size = NativeUtils.getGlobal (_nativeObjectId, name, out valuePtr);

			if (valuePtr != IntPtr.Zero && size > 0)
			{
				LuaObjectDecoder decoder = new LuaObjectDecoder (valuePtr, size, this);
				return decoder.readObject () as LuaValue;
			}

			return new LuaValue();
		}

		/// <summary>
		/// 保留Lua层的变量引用，使其不被GC所回收。
		/// 注：判断value能否被保留取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
		/// 即：LuaValue *val1 = new LuaValue(obj1);与LuaValue *val2 = new LuaValue(obj1);传入方法中效果相同。
		/// </summary>
		/// <param name="value">对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。</param>
		public void retainValue(LuaValue value)
		{
			if (value != null)
			{
				IntPtr valuePtr = IntPtr.Zero;
				LuaObjectEncoder encoder = new LuaObjectEncoder (this);
				encoder.writeObject (value);

				byte[] bytes = encoder.bytes;
				valuePtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, valuePtr, bytes.Length);

				NativeUtils.retainValue (_nativeObjectId, valuePtr);

				if (valuePtr != IntPtr.Zero)
				{
					Marshal.FreeHGlobal (valuePtr);
				}
			}
		}

		/// <summary>
		/// 释放Lua层的变量引用，使其内存管理权交回Lua。
		/// 注：判断value能否被释放取决于value所保存的真实对象，所以只要保证保存对象一致，即使value为不同对象并不影响实际效果。
		/// 即：LuaValue *val1 = new LuaValue(obj1);与LuaValue *val2 = new LuaValue(obj1);传入方法中效果相同。
		/// </summary>
		/// <param name="value">value 对应Lua层变量的原生对象Value，如果value为非Lua回传对象则调用此方法无任何效果。.</param>
		public void releaseValue(LuaValue value)
		{
			if (value != null)
			{
				IntPtr valuePtr = IntPtr.Zero;
				LuaObjectEncoder encoder = new LuaObjectEncoder (this);
				encoder.writeObject (value);

				byte[] bytes = encoder.bytes;
				valuePtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, valuePtr, bytes.Length);

				NativeUtils.releaseValue (_nativeObjectId, valuePtr);

				if (valuePtr != IntPtr.Zero)
				{
					Marshal.FreeHGlobal (valuePtr);
				}
			}
		}

		/// <summary>
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="script">Lua脚本</param>
		public LuaValue evalScript(string script)
		{
			return evalScript (script, null);
		}

		/// <summary>
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值.</returns>
		/// <param name="script">脚本内容.</param>
		/// <param name="scriptController">脚本控制器.</param>
		public LuaValue evalScript(string script, LuaScriptController scriptController)
		{
			IntPtr resultPtr = IntPtr.Zero;

			int scriptControllerId = 0;
			if (scriptController != null)
			{
				scriptControllerId = scriptController.objectId;
			}

			int size = NativeUtils.evalScript (_nativeObjectId, script, scriptControllerId, out resultPtr);
			return LuaObjectDecoder.DecodeObject (resultPtr, size, this) as LuaValue;
		}

		/// <summary>
		/// 从Lua文件中解析脚本
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="filePath">脚本文件路径.</param>
		public LuaValue evalScriptFromFile(string filePath)
		{
			return evalScriptFromFile (filePath, null);
		}

		/// <summary>
		/// 从Lua脚本文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="filePath">Lua脚本文件路径</param>
		/// <param name="scriptController">脚本控制器</param>
		public LuaValue evalScriptFromFile(string filePath, LuaScriptController scriptController)
		{
#if UNITY_ANDROID && !UNITY_EDITOR
			if (!filePath.StartsWith("/") || filePath.StartsWith(Application.streamingAssetsPath, true, null))
			{
				if (filePath.StartsWith (Application.streamingAssetsPath, true, null)) 
				{
					filePath = filePath.Substring(Application.streamingAssetsPath.Length + 1);
				}

				//初始化lua的缓存目录
				setupLuaCacheDir();

				filePath = getLuaCacheFilePath (filePath);
			}

#elif UNITY_EDITOR_WIN

            Regex regex = new Regex("^[a-zA-Z]:/.*");
            if (!regex.IsMatch(filePath))
            {
                //Window下不带盘符的路径为相对路径，需要拼接streamingAssetsPath
                filePath = string.Format("{0}/{1}", Application.streamingAssetsPath, filePath);
            }
#else

            if (!filePath.StartsWith("/"))
			{
				filePath = string.Format("{0}/{1}", Application.streamingAssetsPath, filePath);
			}

#endif

			int scriptControllerId = 0;
			if (scriptController != null)
			{
				scriptControllerId = scriptController.objectId;
			}

			IntPtr resultPtr;
			int size = NativeUtils.evalScriptFromFile (_nativeObjectId, filePath, scriptControllerId, out resultPtr);
			LuaValue retValue = LuaObjectDecoder.DecodeObject (resultPtr, size, this) as LuaValue;

			return retValue;

		}

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="methodName">方法名</param>
		/// <param name="arguments">调用参数列表</param>
		public LuaValue callMethod(string methodName, List<LuaValue> arguments)
		{
			return callMethod (methodName, arguments, null);
		}

		/// <summary>
		/// 调用Lua方法
		/// </summary>
		/// <returns>返回值.</returns>
		/// <param name="methodName">方法名称.</param>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptController">脚本控制器</param>
		public LuaValue callMethod(string methodName, List<LuaValue> arguments, LuaScriptController scriptController)
		{
			int scriptControllerId = 0;
			if (scriptController != null)
			{
				scriptControllerId = scriptController.objectId;
			}

			IntPtr argsPtr = IntPtr.Zero;
			IntPtr resultPtr = IntPtr.Zero;

			if (arguments != null)
			{
				LuaObjectEncoder encoder = new LuaObjectEncoder (this);
				encoder.writeInt32 (arguments.Count);
				foreach (LuaValue value in arguments)
				{
					encoder.writeObject (value);
				}

				byte[] bytes = encoder.bytes;
				argsPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, argsPtr, bytes.Length);
			}

			int size = NativeUtils.callMethod (_nativeObjectId, methodName, argsPtr, scriptControllerId, out resultPtr);

			if (argsPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (argsPtr);
			}

			if (size > 0)
			{
				return LuaObjectDecoder.DecodeObject (resultPtr, size, this) as LuaValue;
			}

			return new LuaValue();
		}

		/// <summary>
		/// 注册Lua方法
		/// </summary>
		/// <param name="methodName">方法名称</param>
		/// <param name="handler">事件处理器</param>
		public void registerMethod(string methodName, LuaMethodHandler handler)
		{
			_methodHandlers [methodName] = handler;

			if (_methodHandleDelegate == null) 
			{
				_methodHandleDelegate = new LuaMethodHandleDelegate(luaMethodRoute);
			}

			IntPtr fp = Marshal.GetFunctionPointerForDelegate(_methodHandleDelegate);
			NativeUtils.registerMethod (_nativeObjectId, methodName, fp);
		}

		/// <summary>
		/// 执行线程
		/// </summary>
		/// <param name="handler">线程处理器.</param>
		/// <param name="arguments">请求参数.</param>
		public void runThread(LuaFunction handler, List<LuaValue> arguments)
		{
			runThread (handler, arguments, null);
		}

		/// <summary>
		/// 执行线程
		/// </summary>
		/// <param name="handler">线程处理器.</param>
		/// <param name="arguments">参数列表.</param>
		/// <param name="scriptController">脚本控制器.</param>
		public void runThread(LuaFunction handler, List<LuaValue> arguments, LuaScriptController scriptController)
		{
			int scriptControllerId = 0;
			if (scriptController != null)
			{
				scriptControllerId = scriptController.objectId;
			}

			IntPtr funcPtr = IntPtr.Zero;
			IntPtr argsPtr = IntPtr.Zero;

			LuaObjectEncoder funcEncoder = new LuaObjectEncoder (this);
			funcEncoder.writeObject (handler);

			byte[] bytes = funcEncoder.bytes;
			funcPtr = Marshal.AllocHGlobal (bytes.Length);
			Marshal.Copy (bytes, 0, funcPtr, bytes.Length);

			if (arguments != null)
			{
				LuaObjectEncoder argEncoder = new LuaObjectEncoder (this);
				argEncoder.writeInt32 (arguments.Count);
				foreach (LuaValue value in arguments)
				{
					argEncoder.writeObject (value);
				}

				bytes = argEncoder.bytes;
				argsPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, argsPtr, bytes.Length);
			}

			NativeUtils.runThread (this.objectId, funcPtr, argsPtr, scriptControllerId);

			if (argsPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (argsPtr);
			}
			if (funcPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (funcPtr);
			}
		}

		/// <summary>
		/// Lua方法处理器
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="methodName">方法名称</param>
		/// <param name="arguments">参数列表</param>
		private IntPtr luaMethodHandler(string methodName, IntPtr args, int size)
		{
			if (_methodHandlers.ContainsKey (methodName))
			{
				//反序列化参数列表
				LuaObjectDecoder decoder = new LuaObjectDecoder(args, size, this);
				int argSize = decoder.readInt32 ();

				List<LuaValue> argumentsList = new List<LuaValue> ();
				for (int i = 0; i < argSize; i++) {

					LuaValue value = decoder.readObject () as LuaValue;
					argumentsList.Add (value);
				}

				LuaMethodHandler handler = _methodHandlers [methodName];
				LuaValue retValue = handler (argumentsList);

				if (retValue == null) 
				{
					retValue = new LuaValue ();
				}

				LuaObjectEncoder encoder = new LuaObjectEncoder (this);
				encoder.writeObject (retValue);

				byte[] bytes = encoder.bytes;
				IntPtr retPtr;
				retPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, retPtr, bytes.Length);

				return retPtr;
			}

			return IntPtr.Zero;
		}

		#if UNITY_ANDROID && !UNITY_EDITOR

		private static bool hasSetupLuaCacheDir = false;

		/// <summary>
		/// 建立Lua的缓存目录，目的是将StreamingAsset中的lua文件拷贝出来。
		/// </summary>
		private void setupLuaCacheDir()
		{
			if (!hasSetupLuaCacheDir) 
			{
				copyLuaFileFromAsset ("");
				hasSetupLuaCacheDir = true;
			}
		}

		/// <summary>
		/// 从Asset中拷贝lua文件
		/// </summary>
		/// <param name="path">目录路径.</param>
		private void copyLuaFileFromAsset(string path)
		{
			AndroidJavaObject currentActivity = UNIEnv.getCurrentActivity ();
			AndroidJavaObject assetManager = currentActivity.Call<AndroidJavaObject> ("getAssets", new object[0]);

			string[] subpaths = assetManager.Call<string[]> ("list", path); 
			if (subpaths.Length > 0) 
			{
				//当前path为目录
				foreach (string subpath in subpaths)
				{
					string targetPath = path == "" ? subpath : string.Format ("{0}/{1}", path, subpath);
					copyLuaFileFromAsset (targetPath);
				}
			} 
			else 
			{
				//当前path为文件
				if (path.ToLower().EndsWith (".lua")) 
				{
					//为lua文件，则进行拷贝
					string destFilePath = getLuaCacheFilePath(path);

					AndroidJavaObject inputStream = assetManager.Call<AndroidJavaObject>("open", path);
					using (MemoryStream ms = new MemoryStream())
					{
						try
						{
							IntPtr buffer = AndroidJNI.NewByteArray(1024);
							jvalue[] args = new jvalue[1];
							args[0].l = buffer;

							IntPtr readMethodId = AndroidJNIHelper.GetMethodID(inputStream.GetRawClass(), "read", "([B)I");
							int hasRead = 0;
							while((hasRead = AndroidJNI.CallIntMethod(inputStream.GetRawObject(), readMethodId, args)) != -1)
							{
								byte[] byteArray = AndroidJNIHelper.ConvertFromJNIArray<byte[]>(buffer);
								ms.Write(byteArray, 0, hasRead);
							}

							ms.Seek(0, SeekOrigin.Begin);
							byte[] bytes = new byte[ms.Length];
							ms.Read(bytes, 0, bytes.Length);

							File.WriteAllBytes(destFilePath, bytes);
						}
						finally
						{
							ms.Close ();
						}
					}
				}
			}
		}

		/// <summary>
		/// 获取lua缓存文件路径
		/// </summary>
		/// <returns>lua文件的绝对路径.</returns>
		/// <param name="subpath">子路径.</param>
		private string getLuaCacheFilePath(string subpath)
		{
			AndroidJavaObject luaCacheDir = getLuaCacheDir ();
			AndroidJavaObject luaFile = new AndroidJavaObject ("java.io.File", string.Format ("{0}/{1}", luaCacheDir.Call<string> ("toString", new object[0]), subpath));

			//判断父级目录是否创建
			AndroidJavaObject luaParentDir = luaFile.Call<AndroidJavaObject>("getParentFile", new object[0]);
			if (!luaParentDir.Call<bool> ("exists", new object[0]))
			{
				luaParentDir.Call<bool> ("mkdirs", new object[0]);
			}

			return luaFile.Call<string> ("toString", new object[0]);

		}

		/// <summary>
		/// 获取Lua文件缓存目录
		/// </summary>
		/// <returns>换粗目录对象.</returns>
		private AndroidJavaObject getLuaCacheDir()
		{
			AndroidJavaObject currentActivity = UNIEnv.getCurrentActivity ();
			AndroidJavaClass EnvironmentClass = new AndroidJavaClass ("android.os.Environment");

			int perm = currentActivity.Call<int> ("checkCallingOrSelfPermission", new object[] { "android.permission.WRITE_EXTERNAL_STORAGE" });

			AndroidJavaObject cacheDir = null;

			if ("mounted" == EnvironmentClass.CallStatic<string> ("getExternalStorageState", new object[0]) && perm == 0)
			{
				cacheDir = currentActivity.Call<AndroidJavaObject> ("getExternalCacheDir", new object[0]);
			}

			if (cacheDir == null)
			{
				cacheDir = currentActivity.Call<AndroidJavaObject> ("getCacheDir", new object[0]);
			}
			
			string cachePath = cacheDir.Call<string>("toString", new object[0]);

			return new AndroidJavaObject ("java.io.File", string.Format ("{0}/lua", cachePath));
		}

		#endif

		/// <summary>
		/// Lua方法路由
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="nativeContextId">lua上下文本地标识</param>
		/// <param name="methodName">方法名称.</param>
		/// <param name="arguments">参数列表缓冲区.</param>
		/// <param name="size">参数列表缓冲区大小.</param>
		[MonoPInvokeCallback (typeof (LuaMethodHandleDelegate))]
		private static IntPtr luaMethodRoute (int nativeContextId, string methodName, IntPtr arguments, int size)
		{
			if (_contexts.ContainsKey (nativeContextId)) 
			{
				LuaContext context = _contexts [nativeContextId].Target as LuaContext;
				if (context != null) 
				{
					return context.luaMethodHandler (methodName, arguments, size);
				}
			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// Lua方法路由
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="classId">类标识</param>
		/// <param name="methodName">方法名称.</param>
		/// <param name="arguments">参数列表缓冲区.</param>
		/// <param name="size">参数列表缓冲区大小.</param>
		[MonoPInvokeCallback (typeof (LuaExceptionHandleDelegate))]
		private static void luaException (int contextId, string errorMessage)
		{
			if (_contexts.ContainsKey (contextId))
			{
				LuaContext context = _contexts [contextId].Target as LuaContext;
				if (context != null && context._exceptionHandler != null)
				{
					context._exceptionHandler (errorMessage);
				}
			}
		}

		/// <summary>
		/// 当前上下文对象
		/// </summary>
		private static LuaContext _currentContext;

		/// <summary>
		/// 获取当前上下文对象
		/// </summary>
		/// <value>Lua上下文对象.</value>
		public static LuaContext currentContext
		{
			get
			{
				if (_currentContext == null) 
				{
					_currentContext = new LuaContext();
				}

				return _currentContext;
			}
		}
	}
}