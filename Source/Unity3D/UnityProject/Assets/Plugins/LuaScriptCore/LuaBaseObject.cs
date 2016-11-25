using System.Collections;

namespace cn.vimfung.luascriptcore
{
	public class LuaBaseObject
	{
		protected int _nativeObjectId;

		/// <summary>
		/// 初始化LuaBaseObject
		/// </summary>
		public LuaBaseObject ()
		{
			
		}

		/// <summary>
		/// 初始化LuaBaseObject
		/// </summary>
		/// <param name="decoder">对象解码器</param>
		public LuaBaseObject (LuaObjectDecoder decoder)
		{

		}

		/// <summary>
		/// 释放LuaBaseObject
		/// </summary>
		~LuaBaseObject ()
		{
			NativeUtils.releaseObject(_nativeObjectId);
		}

//		/// <summary>
//		/// 获取原生类名称
//		/// </summary>
//		/// <value>类名称.</value>
//		public virtual string nativeClassName
//		{
//			get 
//			{
//				return "Object";
//			}
//		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public virtual void serialization(LuaObjectEncoder encoder)
		{
			encoder.writeByte ((byte)'L');
			encoder.writeString (this.GetType().Name);
			encoder.writeByte ((byte)';');
		}
	}
}