using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 脚本控制器
	/// </summary>
	public class LuaScriptController : LuaBaseObject
	{
		/// <summary>
		/// 初始化
		/// </summary>
		public LuaScriptController()
		{
			_nativeObjectId = NativeUtils.createLuaScriptController ();
		}

		/// <summary>
		/// 设置脚本执行超时时间
		/// </summary>
		/// <param name="timeout">单位：秒，设置0表示没有时间限制</param>
		public void setTimeout(int timeout)
		{
			NativeUtils.scriptControllerSetTimeout (_nativeObjectId, timeout);
		}

		/// <summary>
		/// 强制退出执行脚本
		/// </summary>
		public void forceExit()
		{
			NativeUtils.scriptControllerForceExit (_nativeObjectId);
		}
	}

}
