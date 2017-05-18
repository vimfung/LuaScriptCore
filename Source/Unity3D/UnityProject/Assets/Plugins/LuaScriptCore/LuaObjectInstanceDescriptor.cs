using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore;
using System;
using System.Runtime.InteropServices;

namespace cn.vimfung.luascriptcore.modules.oo
{
	/// <summary>
	/// 实例对象描述器
	/// </summary>
	public class LuaObjectInstanceDescriptor : LuaObjectDescriptor 
	{
		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="obj">对象</param>
		public LuaObjectInstanceDescriptor(object obj)
			:base(obj)
		{
			
		}

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="decode">解码器.</param>
		public LuaObjectInstanceDescriptor(LuaObjectDecoder decode)
			:base(decode)
		{
			
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization(LuaObjectEncoder encoder)
		{
			base.serialization (encoder);
			encoder.writeString (obj.GetType ().Name);
		}
	}
}
