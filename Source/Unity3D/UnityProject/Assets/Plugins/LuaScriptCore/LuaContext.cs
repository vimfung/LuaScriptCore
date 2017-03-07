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

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// Lua方法处理器
	/// </summary>
	public delegate LuaValue LuaMethodHandler (List<LuaValue> arguments);

	/// <summary>
	/// 异常处理器
	/// </summary>
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
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
		/// 创建LuaContext
		/// </summary>
		public LuaContext()
		{
			_methodHandlers = new Dictionary<string, LuaMethodHandler> ();
			_nativeObjectId = NativeUtils.createLuaContext ();
			_contexts.Add (_nativeObjectId, new WeakReference(this));

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
			NativeUtils.addSearchPath (_nativeObjectId, path + "/?.lua");
		}

		/// <summary>
		/// 异常时触发
		/// </summary>
		/// <param name="handler">事件处理器</param>
		public void onException(LuaExceptionHandler handler)
		{
			IntPtr fp = Marshal.GetFunctionPointerForDelegate(handler);
			NativeUtils.setExceptionHandler (_nativeObjectId, fp);
		}

		/// <summary>
		/// 解析Lua脚本
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="script">Lua脚本</param>
		public LuaValue evalScript(string script)
		{
			IntPtr resultPtr = IntPtr.Zero;
			int size = NativeUtils.evalScript (_nativeObjectId, script, out resultPtr);
			return LuaObjectDecoder.DecodeObject (resultPtr, size) as LuaValue;
		}

		/// <summary>
		/// 从Lua脚本文件中解析Lua脚本
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="filePath">Lua脚本文件路径</param>
		public LuaValue evalScriptFromFile(string filePath)
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
			IntPtr resultPtr;
			int size = NativeUtils.evalScriptFromFile (_nativeObjectId, filePath, out resultPtr);
			LuaValue retValue = LuaObjectDecoder.DecodeObject (resultPtr, size) as LuaValue;

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
			IntPtr argsPtr = IntPtr.Zero;
			IntPtr resultPtr = IntPtr.Zero;

			if (arguments != null)
			{
				LuaObjectEncoder encoder = new LuaObjectEncoder ();
				encoder.writeInt32 (arguments.Count);
				foreach (LuaValue value in arguments)
				{
					encoder.writeObject (value);
				}

				byte[] bytes = encoder.bytes;
				argsPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, argsPtr, bytes.Length);
			}

			int size = NativeUtils.callMethod (_nativeObjectId, methodName, argsPtr, out resultPtr);

			if (argsPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (argsPtr);
			}
				
			if (size > 0)
			{
				return LuaObjectDecoder.DecodeObject (resultPtr, size) as LuaValue;
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
		/// 注册模块
		/// </summary>
		/// <typeparam name="T">模块类型</typeparam>
		public void registerModule<T>()
			where T : LuaModule
		{
			LuaModule.register(this, typeof(T));
		}

		/// <summary>
		/// 判断模块是否注册
		/// </summary>
		/// <returns>true表示已注册,false表示尚未注册</returns>
		/// <param name="moduleName">模块名称.</param>
		public bool isModuleRegisted(string moduleName)
		{
			return NativeUtils.isModuleRegisted(_nativeObjectId, moduleName);
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
				LuaObjectDecoder decoder = new LuaObjectDecoder(args, size);
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

				LuaObjectEncoder encoder = new LuaObjectEncoder ();
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
		/// 获取当前的Activity
		/// </summary>
		/// <returns>当前的Activity.</returns>
		private AndroidJavaObject getCurrentActivity()
		{
			AndroidJavaClass unityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer");
			AndroidJavaObject currentActivity  = unityPlayer.GetStatic<AndroidJavaObject>("currentActivity");
			return currentActivity;
		}

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
			AndroidJavaObject currentActivity = getCurrentActivity ();
			AndroidJavaObject assetManager = currentActivity.Call<AndroidJavaObject> ("getAssets", new object[0]);

			string[] subpaths = assetManager.Call<string[]> ("list", path); 
			if (subpaths.Length > 0) 
			{
				//当前path为目录
				foreach (string subpath in subpaths)
				{
					copyLuaFileFromAsset (string.Format ("{0}{1}/", path, subpath));
				}
			} 
			else 
			{
				//当前path为文件
				string fileName = path.Substring(0, path.Length - 1);
				if (fileName.ToLower().EndsWith (".lua")) 
				{
					//为lua文件，则进行拷贝
					string destFilePath = getLuaCacheFilePath(fileName);

					AndroidJavaObject inputStream = assetManager.Call<AndroidJavaObject>("open", fileName);
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
			AndroidJavaObject currentActivity = getCurrentActivity ();
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

			return new AndroidJavaObject ("java.io.File", string.Format ("{0}/lua", cacheDir.Call<string>("toString", new object[0])));
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