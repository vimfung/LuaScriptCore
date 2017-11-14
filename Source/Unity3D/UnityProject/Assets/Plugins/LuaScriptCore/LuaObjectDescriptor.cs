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
		/// 原生类型标识
		/// </summary>
		private int _nativeTypeId;

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
			_luaObjectId = obj.GetHashCode ().ToString ("x8");
			_nativeTypeId = LuaExportsTypeManager.defaultManager.getNativeTypeId (obj.GetType ());
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

			_nativeTypeId = decoder.readInt32 ();
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
			encoder.writeInt32 (_nativeTypeId);

			//写入自定义数据
			encoder.writeInt32(1);
			encoder.writeString ("NativeClass");
			encoder.writeString (this.obj.GetType().FullName);
		}
	}
}
