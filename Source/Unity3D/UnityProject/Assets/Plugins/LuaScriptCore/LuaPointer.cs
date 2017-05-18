using System;
using System.Runtime.InteropServices;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// Lua的指针类型
	/// </summary>
	public class LuaPointer : LuaBaseObject
	{
		public LuaPointer (object obj)
		{
			if (obj != null)
			{
				_objReference = new LuaObjectReference (obj);
			}
		}

		private LuaObjectReference _objReference = null;

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="decoder">对象解码器</param>
		public LuaPointer (LuaObjectDecoder decoder)
			: base(decoder)
		{
			Int64 objRefId = decoder.readInt64 ();
			_objReference = LuaObjectReference.findObject (objRefId);
			luaObjectId = decoder.readString ();
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization(LuaObjectEncoder encoder)
		{
			base.serialization (encoder);
			encoder.writeInt64 (_objReference.referenceId);
			encoder.writeString (luaObjectId);
		}
	}


}

