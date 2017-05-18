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
		private LuaObjectReference _objRef;

		/// <summary>
		/// 获取对象
		/// </summary>
		/// <value>对象</value>
		public object obj
		{
			get
			{
				return _objRef.target;
			}
		}

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="obj">对象</param>
		public LuaObjectDescriptor(object obj)
		{
			_objRef = new LuaObjectReference(obj);
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
			Int64 objRefId = decoder.readInt64 ();
			_objRef = LuaObjectReference.findObject (objRefId);

			luaObjectId = decoder.readString ();
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization(LuaObjectEncoder encoder)
		{
			base.serialization (encoder);

			encoder.writeInt64 (_objRef.referenceId);
			encoder.writeString (luaObjectId);

			//写入自定义数据
			encoder.writeInt32(1);
			encoder.writeString ("NativeClass");
			encoder.writeString (this.obj.GetType().FullName);
		}
	}
}
