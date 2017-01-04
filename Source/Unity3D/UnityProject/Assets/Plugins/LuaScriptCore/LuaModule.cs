using System.Collections;
using System.Reflection;
using System;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// Lua模块，在模块下定义的类方法可以导出到lua中进行使用。
	/// </summary>
	public class LuaModule : object
	{
		/// <summary>
		/// 导出模块方法集合
		/// </summary>
		private static Dictionary<int, Dictionary<string, MethodInfo>> _exportsModuleMethods = new Dictionary<int, Dictionary<string, MethodInfo>> (); 

		/// <summary>
		/// 方法处理委托
		/// </summary>
		private static LuaModuleMethodHandleDelegate _moduleMethodHandleDelegate;

		/// <summary>
		/// 获取模块版本
		/// </summary>
		/// <value>版本号.</value>
		public static string version<T> ()
		{
			return "";
		}

		/// <summary>
		/// 获取模块名称
		/// </summary>
		/// <value>模块名称.</value>
		public static string moduleName<T> ()
		{
			Type t = typeof(T);
			return t.Name;
		}

		/// <summary>
		/// 注册模块
		/// </summary>
		internal static void register<T> (LuaContext context)
		{
			string moduleName = LuaModule.moduleName<T>();
			if (context.isModuleRegisted(moduleName))
			{
				return;
			}

			//初始化模块导出信息
			Dictionary<string, MethodInfo> exportMethods = new Dictionary<string, MethodInfo> ();
			List<string> exportMethodNames = new List<string> ();

			MethodInfo[] methods = typeof(T).GetMethods();
			foreach (MethodInfo m in methods) 
			{
				if (m.IsStatic && m.IsPublic)
				{
					//静态和公开的方法会导出到Lua
					exportMethodNames.Add(m.Name);
					exportMethods.Add (m.Name, m);
				}
			}

			LuaObjectEncoder encoder = new LuaObjectEncoder ();
			encoder.writeInt32 (exportMethodNames.Count);
			foreach (string name in exportMethodNames) 
			{
				encoder.writeString (name);
			}

			byte[] bytes = encoder.bytes;
			IntPtr exportMethodNamesPtr;
			exportMethodNamesPtr = Marshal.AllocHGlobal (bytes.Length);
			Marshal.Copy (bytes, 0, exportMethodNamesPtr, bytes.Length);

			if (_moduleMethodHandleDelegate == null) 
			{
				_moduleMethodHandleDelegate = new LuaModuleMethodHandleDelegate(luaModuleMethodRoute);
			}
			IntPtr fp = Marshal.GetFunctionPointerForDelegate(_moduleMethodHandleDelegate);
		
			//注册模块
			int moduleId = NativeUtils.registerModule (context.objectId, moduleName, exportMethodNamesPtr, fp);

			//关联注册模块的注册方法
			_exportsModuleMethods.Add (moduleId, exportMethods);
		}

		/// <summary>
		/// Lua方法路由
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="nativeContextId">lua上下文本地标识</param>
		/// <param name="methodName">方法名称.</param>
		/// <param name="arguments">参数列表缓冲区.</param>
		/// <param name="size">参数列表缓冲区大小.</param>
		[MonoPInvokeCallback (typeof (LuaModuleMethodHandleDelegate))]
		private static IntPtr luaModuleMethodRoute (int nativeModuleId, string methodName, IntPtr arguments, int size)
		{
			if (_exportsModuleMethods.ContainsKey (nativeModuleId)) 
			{
				Dictionary<string, MethodInfo> exportMethods = _exportsModuleMethods [nativeModuleId];
				if (exportMethods.ContainsKey (methodName)) 
				{
					//存在该方法, 进行调用
					MethodInfo m = exportMethods [methodName];
						
					ArrayList argsArr = parseMethodParameters (m, arguments, size);
					object ret = m.Invoke (null, argsArr != null ? argsArr.ToArray() : null);
					LuaValue retValue = new LuaValue (ret);

					LuaObjectEncoder encoder = new LuaObjectEncoder ();
					encoder.writeObject (retValue);

					byte[] bytes = encoder.bytes;
					IntPtr retPtr;
					retPtr = Marshal.AllocHGlobal (bytes.Length);
					Marshal.Copy (bytes, 0, retPtr, bytes.Length);

					return retPtr;
				}
			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// 解析方法的参数列表
		/// </summary>
		/// <returns>参数列表</returns>
		/// <param name="m">方法信息</param>
		/// <param name="arguments">参数列表数据</param>
		/// <param name="size">参数列表数据长度</param>
		private static ArrayList parseMethodParameters(MethodInfo m, IntPtr arguments, int size)
		{
			List<LuaValue> argumentsList = null;
			if (arguments != IntPtr.Zero) 
			{
				//反序列化参数列表
				LuaObjectDecoder decoder = new LuaObjectDecoder(arguments, size);
				int argSize = decoder.readInt32 ();

				argumentsList = new List<LuaValue> ();
				for (int i = 0; i < argSize; i++) {

					LuaValue value = decoder.readObject () as LuaValue;
					argumentsList.Add (value);
				}
			}

			ArrayList argsArr = null;
			ParameterInfo[] parameters = m.GetParameters ();
			if (parameters.Length > 0 && argumentsList != null) 
			{
				int i = 0;
				argsArr = new ArrayList ();
				foreach (ParameterInfo p in parameters) 
				{
					if (i >= argumentsList.Count) 
					{
						break;
					}

					object value = null;
					if (p.ParameterType == typeof(Int32)
						|| p.ParameterType == typeof(Int64)
						|| p.ParameterType == typeof(Int16)
						|| p.ParameterType == typeof(UInt16)
						|| p.ParameterType == typeof(UInt32)
						|| p.ParameterType == typeof(UInt64)) 
					{
						value = argumentsList [i].toInteger ();
					} 
					else if (p.ParameterType == typeof(double)
						|| p.ParameterType == typeof(float)) 
					{
						value = argumentsList [i].toNumber ();
					} 
					else if (p.ParameterType == typeof(bool))
					{
						value = argumentsList [i].toBoolean ();
					}
					else if (p.ParameterType == typeof(string)) 
					{
						value = argumentsList [i].toString ();
					}
					else if (p.ParameterType.IsArray) 
					{
						//数组
						if (p.ParameterType == typeof(byte[])) 
						{
							//二进制数组
							value = argumentsList[i].toData();
						}
						else if (p.ParameterType == typeof(Int32[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<Int32> intArr = new List<Int32> ();
							foreach (LuaValue item in arr)
							{
								intArr.Add (item.toInteger ());
							}
							value = intArr.ToArray ();
						}
						else if (p.ParameterType == typeof(Int64[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<Int64> intArr = new List<Int64> ();
							foreach (LuaValue item in arr)
							{
								intArr.Add (item.toInteger ());
							}
							value = intArr.ToArray ();
						}
						else if (p.ParameterType == typeof(Int16[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<Int16> intArr = new List<Int16> ();
							foreach (LuaValue item in arr)
							{
								intArr.Add (Convert.ToInt16(item.toInteger ()));
							}
							value = intArr.ToArray ();
						}
						else if (p.ParameterType == typeof(UInt32[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<UInt32> intArr = new List<UInt32> ();
							foreach (LuaValue item in arr)
							{
								intArr.Add (Convert.ToUInt32(item.toInteger ()));
							}
							value = intArr.ToArray ();
						}
						else if (p.ParameterType == typeof(UInt64[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<UInt64> intArr = new List<UInt64> ();
							foreach (LuaValue item in arr)
							{
								intArr.Add (Convert.ToUInt64(item.toInteger ()));
							}
							value = intArr.ToArray ();
						}
						else if (p.ParameterType == typeof(UInt16[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<UInt16> intArr = new List<UInt16> ();
							foreach (LuaValue item in arr)
							{
								intArr.Add (Convert.ToUInt16(item.toInteger ()));
							}
							value = intArr.ToArray ();
						}
						else if (p.ParameterType == typeof(bool[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<bool> boolArr = new List<bool> ();
							foreach (LuaValue item in arr)
							{
								boolArr.Add (Convert.ToBoolean(item.toInteger ()));
							}
							value = boolArr.ToArray ();
						}
						else if (p.ParameterType == typeof(double[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<double> doubleArr = new List<double> ();
							foreach (LuaValue item in arr)
							{
								doubleArr.Add (item.toNumber());
							}
							value = doubleArr.ToArray ();
						}
						else if (p.ParameterType == typeof(float[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<float> floatArr = new List<float> ();
							foreach (LuaValue item in arr)
							{
								floatArr.Add ((float)item.toNumber());
							}
							value = floatArr.ToArray ();
						}
						else if (p.ParameterType == typeof(string[]))
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							List<string> floatArr = new List<string> ();
							foreach (LuaValue item in arr)
							{
								floatArr.Add (item.toString());
							}
							value = floatArr.ToArray ();
						}
						else
						{
							List<LuaValue> arr = argumentsList[i].toArray();
							ArrayList objArr = new ArrayList ();
							foreach (LuaValue item in arr)
							{
								objArr.Add (item.toObject());
							}
							value = objArr.ToArray ();
						}
					}
					else if (p.ParameterType == typeof(Hashtable)) 
					{
						//字典
						Dictionary<string, LuaValue> map = argumentsList[i].toMap();
						if (p.ParameterType == typeof(Dictionary<string, int>)) 
						{
							Dictionary<string, int> dict = new Dictionary<string, int> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, kv.Value.toInteger ());
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, int>)) 
						{
							Dictionary<string, int> dict = new Dictionary<string, int> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, kv.Value.toInteger ());
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, uint>)) 
						{
							Dictionary<string, uint> dict = new Dictionary<string, uint> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, Convert.ToUInt32(kv.Value.toInteger ()));
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, Int16>)) 
						{
							Dictionary<string, Int16> dict = new Dictionary<string, Int16> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, Convert.ToInt16(kv.Value.toInteger ()));
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, UInt16>)) 
						{
							Dictionary<string, UInt16> dict = new Dictionary<string, UInt16> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, Convert.ToUInt16(kv.Value.toInteger ()));
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, Int64>)) 
						{
							Dictionary<string, Int64> dict = new Dictionary<string, Int64> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, Convert.ToInt64(kv.Value.toInteger ()));
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, UInt64>)) 
						{
							Dictionary<string, UInt64> dict = new Dictionary<string, UInt64> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, Convert.ToUInt64(kv.Value.toInteger ()));
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, bool>)) 
						{
							Dictionary<string, bool> dict = new Dictionary<string, bool> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, kv.Value.toBoolean ());
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, double>)) 
						{
							Dictionary<string, double> dict = new Dictionary<string, double> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, kv.Value.toNumber());
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, float>)) 
						{
							Dictionary<string, float> dict = new Dictionary<string, float> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, (float)kv.Value.toNumber());
							}
							value = dict;
						}
						else if (p.ParameterType == typeof(Dictionary<string, string>)) 
						{
							Dictionary<string, string> dict = new Dictionary<string, string> ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, kv.Value.toString());
							}
							value = dict;
						}
						else
						{
							Hashtable dict = new Hashtable ();
							foreach (KeyValuePair<string, LuaValue> kv in map) 
							{
								dict.Add (kv.Key, kv.Value.toObject());
							}
							value = dict;
						}
					}
					else
					{
						value = argumentsList [i].toObject ();
					}

					argsArr.Add (value);

					i++;
				}
			}

			return argsArr;
		}
	}
}
