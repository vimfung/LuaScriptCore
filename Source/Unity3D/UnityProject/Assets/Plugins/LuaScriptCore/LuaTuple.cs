using System;
using System.Collections.Generic;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 元组，仅用于返回多个值到Lua中使用，同时，Lua中的多个返回值会转换为该类型对象返回原生层。
	/// </summary>
	public class LuaTuple : LuaBaseObject
	{
		/// <summary>
		/// 初始化
		/// </summary>
		public LuaTuple ()
		{
			
		}

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="decoder">对象解码器</param>
		public LuaTuple (LuaObjectDecoder decoder)
			: base(decoder)
		{
			int size = decoder.readInt32 ();
			if (size > 0) 
			{
				for (int i = 0; i < size; i++)
				{
					LuaValue item = decoder.readObject () as LuaValue;
					if (item != null) 
					{
						_returnValues.Add (item);
					}
				}
			}
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization(LuaObjectEncoder encoder)
		{
			base.serialization (encoder);

			encoder.writeInt32 (_returnValues.Count);
			foreach (LuaValue value in _returnValues) 
			{
				encoder.writeObject (value);
			}
		}

		private List<LuaValue> _returnValues = new List<LuaValue> ();

		/// <summary>
		/// 获取返回值数量
		/// </summary>
		/// <value>返回值数量.</value>
		public int count
		{
			get
			{
				return _returnValues.Count;
			}
		}

		/// <summary>
		/// 添加返回值
		/// </summary>
		/// <param name="value">返回值.</param>
		public void addRetrunValue(object value)
		{
			_returnValues.Add (new LuaValue (value));
		}

		/// <summary>
		/// 获取返回值
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="index">位置索引.</param>
		public object getReturnValueByIndex(int index)
		{
			return _returnValues [index];
		}
	}
}

