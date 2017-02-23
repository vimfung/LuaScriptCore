using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 对象描述器
	/// </summary>
	public class LuaObjectDescriptor : LuaBaseObject
	{
		private object _obj;

		/// <summary>
		/// 获取对象
		/// </summary>
		/// <value>对象</value>
		public object obj
		{
			get
			{
				return _obj;
			}
		}

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="obj">对象</param>
		public LuaObjectDescriptor(object obj)
		{
			_obj = obj;
			if (obj is LuaBaseObject)
			{
				_luaObjectId = (obj as LuaBaseObject).luaObjectId;
			}
		}

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="decoder">对象解码器</param>
		public LuaObjectDescriptor (LuaObjectDecoder decoder)
			: base(decoder)
		{
			IntPtr ptr = new IntPtr (decoder.readInt64 ());
			_obj = Marshal.GetObjectForIUnknown (ptr);

			_luaObjectId = decoder.readString ();
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization(LuaObjectEncoder encoder)
		{
			base.serialization (encoder);

			IntPtr ptr = Marshal.GetIUnknownForObject (obj);
			encoder.writeInt64 (ptr.ToInt64 ());
			encoder.writeString (luaObjectId != null ? luaObjectId : "");
		}
	}
}
