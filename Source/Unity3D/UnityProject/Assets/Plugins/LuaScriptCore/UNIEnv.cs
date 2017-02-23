using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore;
using System.Runtime.InteropServices;
using AOT;
using System;
using System.Collections.Generic;
using cn.vimfung.luascriptcore.modules.oo;

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
	}

	/// <summary>
	/// 设置对象ID
	/// </summary>
	/// <param name="instance">实例对象</param>
	/// <param name="nativeObjectId">原生对象标识</param>
	[MonoPInvokeCallback (typeof (LuaSetNativeObjectIdHandleDelegate))]
	private static void setNativeObjectId (IntPtr instance, int nativeObjectId, string luaObjectId)
	{
		if (instance != IntPtr.Zero)
		{
			LuaBaseObject luaObj = Marshal.GetObjectForIUnknown (instance) as LuaBaseObject;
			luaObj.objectId = nativeObjectId;
			luaObj.luaObjectId = luaObjectId;
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
}
