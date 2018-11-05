using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore;
using System.Runtime.InteropServices;
using AOT;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text.RegularExpressions;

namespace cn.vimfung.luascriptcore
{
	public class UNIEnv : object 
	{
		/// <summary>
		/// 设置原生对象ID委托
		/// </summary>
		private static LuaSetNativeObjectIdHandleDelegate _setNativeObjectIdDelegate;

		/// <summary>
		/// 根据实例获取类名称
		/// </summary>
		private static LuaGetClassNameByInstanceDelegate _getClassByInstanceDelegate;

		/// <summary>
		/// 导出原生类型委托
		/// </summary>
		private static LuaExportsNativeTypeDelegate _exportsNativeTypeDelegate;

		/// <summary>
		/// 名称空间集合
		/// </summary>
		private static HashSet<Assembly> _assemblys;

		/// <summary>
		/// 排除检测的程序集
		/// </summary>
		private static List<Regex> _excludeAssemblyNames = new List<Regex> () {
			new Regex("^System$", RegexOptions.IgnoreCase),
			new Regex("^System[.].+", RegexOptions.IgnoreCase),
			new Regex("^Boo$", RegexOptions.IgnoreCase),
			new Regex("^Boo[.].+", RegexOptions.IgnoreCase),
			new Regex("^Unity$", RegexOptions.IgnoreCase),
			new Regex("^Unity[.].+", RegexOptions.IgnoreCase),
			new Regex("^ICSharpCode$", RegexOptions.IgnoreCase),
			new Regex("^ICSharpCode[.].+", RegexOptions.IgnoreCase),
			new Regex("^UnityEngine$", RegexOptions.IgnoreCase),
			new Regex("^UnityEngine[.].+", RegexOptions.IgnoreCase),
			new Regex("^UnityScript$", RegexOptions.IgnoreCase),
			new Regex("^UnityScript[.].+", RegexOptions.IgnoreCase),
			new Regex("^Mono$", RegexOptions.IgnoreCase),
			new Regex("^Mono[.].+", RegexOptions.IgnoreCase),
			new Regex("^UnityEditor$", RegexOptions.IgnoreCase),
			new Regex("^UnityEditor[.].+", RegexOptions.IgnoreCase),
			new Regex("^nunit.framework$", RegexOptions.IgnoreCase),
			new Regex("^mscorlib$", RegexOptions.IgnoreCase),
		};

		/// <summary>
		/// 初始化
		/// </summary>
		public static void setup()
		{
			if (_setNativeObjectIdDelegate == null)
			{
				_setNativeObjectIdDelegate = new LuaSetNativeObjectIdHandleDelegate (setNativeObjectId);
			}
			NativeUtils.bindSetNativeObjectIdHandler (Marshal.GetFunctionPointerForDelegate(_setNativeObjectIdDelegate));

			if (_getClassByInstanceDelegate == null)
			{
				_getClassByInstanceDelegate = new LuaGetClassNameByInstanceDelegate (getClassByInstance);
			}
			NativeUtils.bindGetClassNameByInstanceHandler (Marshal.GetFunctionPointerForDelegate (_getClassByInstanceDelegate));

			if (_exportsNativeTypeDelegate == null)
			{
				_exportsNativeTypeDelegate = new LuaExportsNativeTypeDelegate (exportsNativeType);
			}
			NativeUtils.bindExportsNativeTypeHandler (Marshal.GetFunctionPointerForDelegate (_exportsNativeTypeDelegate));

		}

		/// <summary>
		/// 获取当前的Activity
		/// </summary>
		/// <returns>当前的Activity.</returns>
		public static AndroidJavaObject getCurrentActivity()
		{
			AndroidJavaClass unityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer");
			AndroidJavaObject currentActivity  = unityPlayer.GetStatic<AndroidJavaObject>("currentActivity");
			return currentActivity;
		}

		/// <summary>
		/// 设置对象ID
		/// </summary>
		/// <param name="instance">实例对象</param>
		/// <param name="nativeObjectId">原生对象标识</param>
		[MonoPInvokeCallback (typeof (LuaSetNativeObjectIdHandleDelegate))]
		private static void setNativeObjectId (Int64 instance, int nativeObjectId, string luaObjectId)
		{
			if (instance != 0)
			{
				LuaObjectReference objRef = LuaObjectReference.findObject (instance);
				LuaBaseObject luaObj = objRef.target as LuaBaseObject;
				if (luaObj != null)
				{
					luaObj.objectId = nativeObjectId;
					luaObj.luaObjectId = luaObjectId;
				}
			}	
		}

		/// <summary>
		/// 根据实例获取类型名称
		/// </summary>
		/// <returns>实例的类型名称</returns>
		/// <param name="objPtr">对象指针</param>
		[MonoPInvokeCallback (typeof (LuaGetClassNameByInstanceDelegate))]
		private static string getClassByInstance (IntPtr objPtr)
		{
			if (objPtr != IntPtr.Zero)
			{
				object obj = Marshal.GetObjectForIUnknown (objPtr);
				return obj.GetType ().Name;
			}

			return null;
		}

		/// <summary>
		/// 导出原生类型
		/// </summary>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="typeName">类型名称.</param>
		[MonoPInvokeCallback (typeof (LuaExportsNativeTypeDelegate))]
		private static void exportsNativeType(int contextId, string typeName)
		{
			if (_assemblys == null)
			{
				//初始化名称空间
				_assemblys = new HashSet<Assembly> ();

				Assembly[] assemblys = AppDomain.CurrentDomain.GetAssemblies ();
				foreach (Assembly assembly in assemblys)
				{
					bool needAdd = true;
					string assemblyName = assembly.GetName().Name;
					foreach (Regex regex in _excludeAssemblyNames)
					{
						if (regex.IsMatch (assemblyName))
						{
							needAdd = false;
							break;
						}
					}

					if (needAdd)
					{
						_assemblys.Add (assembly);
					}
				}
			}
				
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				//查找类型
				Type targetType = null;
				string fullTypeName = typeName.Replace ("_", ".");
				foreach (Assembly assembly in _assemblys)
				{
					targetType = Type.GetType(string.Format("{0},{1}", typeName, assembly.GetName().Name));
					if (targetType == null)
					{
						targetType = Type.GetType(string.Format("{0},{1}", fullTypeName, assembly.GetName().Name));

						if (targetType == null)
						{
							foreach (Type type in assembly.GetTypes())
							{
								if (type.Name == typeName)
								{
									targetType = type;
									break;
								}
							}
						}
					}

					if (targetType != null)
					{
						break;
					}

				}
					
				if (targetType != null)
				{
					//为导出类型
					context.exportsNativeType(targetType);
				}
			}
		}
	}

}