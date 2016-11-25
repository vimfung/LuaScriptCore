using System.Collections;
using System;
using System.Collections.Generic;

namespace cn.vimfung.luascriptcore
{
	public enum LuaValueType
	{
		Nil = 0,
		Number = 1,
		Boolean = 2,
		String = 3,
		Array = 4,
		Map = 5,
		Ptr = 6,
		Object = 7,
		Integer = 8,
		Data = 9,
		Function = 10
	};

	public class LuaValue : LuaBaseObject
	{
		private LuaValueType _type;
		private object _value;

		/// <summary>
		/// 初始化一个Nil值
		/// </summary>
		public LuaValue()
		{
			_value = null;
			_type = LuaValueType.Nil;
		}

		/// <summary>
		/// 初始化一个整型值
		/// </summary>
		/// <param name="value">整型值</param>
		public LuaValue(int value)
		{
			_value = value;
			_type = LuaValueType.Integer;
		}

		/// <summary>
		/// 初始化一个双精度浮点数
		/// </summary>
		/// <param name="value">浮点数</param>
		public LuaValue(double value)
		{
			_value = value;
			_type = LuaValueType.Number;
		}

		/// <summary>
		/// 初始化一个布尔值
		/// </summary>
		/// <param name="value">布尔值</param>
		public LuaValue(bool value)
		{
			_value = value;
			_type = LuaValueType.Boolean;
		}

		/// <summary>
		/// 初始化一个字符串值
		/// </summary>
		/// <param name="value">字符串.</param>
		public LuaValue(string value)
		{
			_value = value;
			_type = LuaValueType.String;
		}

		/// <summary>
		/// 初始化一个数组值
		/// </summary>
		/// <param name="value">数组</param>
		public LuaValue(List<LuaValue> value)
		{
			_value = value;
			_type = LuaValueType.Array;
		}

		/// <summary>
		/// 初始化一个字典值
		/// </summary>
		/// <param name="value">字典.</param>
		public LuaValue(Dictionary<string, LuaValue> value)
		{
			_value = value;
			_type = LuaValueType.Map;
		}

		/// <summary>
		/// 初始化一个二进制数组值
		/// </summary>
		/// <param name="value">二进制数组.</param>
		public LuaValue(byte[] value)
		{
			_value = value;
			_type = LuaValueType.Data;
		}

		/// <summary>
		/// 读取一个数组列表
		/// </summary>
		/// <returns>数组列表</returns>
		/// <param name="decoder">对象解码器</param>
		private List<LuaValue> readArrayList(LuaObjectDecoder decoder)
		{
			int size = decoder.readInt32 ();
			if (size > 0) 
			{
				List<LuaValue> list = new List<LuaValue> ();
				for (int i = 0; i < size; i++)
				{
					LuaValue item = decoder.readObject () as LuaValue;
					if (item != null) 
					{
						list.Add (item);
					}
				}

				return list;
			}

			return null;
		}

		/// <summary>
		/// 读取一个字典
		/// </summary>
		/// <returns>字典对象</returns>
		/// <param name="decoder">对象解码器</param>
		private Dictionary<string, LuaValue> readHashtable(LuaObjectDecoder decoder)
		{
			int size = decoder.readInt32 ();
			if (size > 0) 
			{
				Dictionary<string, LuaValue> table = new Dictionary<string, LuaValue> ();
				for (int i = 0; i < size; i++) 
				{
					string key = decoder.readString ();
					LuaValue item = decoder.readObject () as LuaValue;
					if (key != null && item != null) 
					{
						table.Add (key, item);
					}
				}

				return table;
			}

			return null;
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization (LuaObjectEncoder encoder)
		{
			base.serialization (encoder);

			encoder.writeInt16 ((Int16)type);

			switch (type)
			{
			case LuaValueType.Number:
				{
					encoder.writeDouble (toNumber ());
					break;
				}
			case LuaValueType.Integer:
				{
					encoder.writeInt32 (toInteger ());
					break;
				}
			case LuaValueType.String:
				{
					encoder.writeString (toString ());
					break;
				}
			case LuaValueType.Data:
				{
					encoder.writeBytes (toData ());
					break;
				}
			case LuaValueType.Array:
				{
					List<LuaValue> list = toArray ();

					encoder.writeInt32 (list.Count);
					foreach (LuaValue value in list) 
					{
						value.serialization (encoder);
					}
					break;
				}
			case LuaValueType.Map:
				{
					Dictionary<string, LuaValue> map = toMap();
					encoder.writeInt32 (map.Count);

					foreach (KeyValuePair<string, LuaValue> kv in map) 
					{
						encoder.writeString (kv.Key);
						encoder.writeObject (kv.Value);
					}
					break;
				}
			case LuaValueType.Object:
				{
//					toObject() -> serialization(NULL, encoder);
					break;
				}
			case LuaValueType.Boolean:
				{
					encoder.writeByte (Convert.ToByte(toBoolean ()));
					break;
				}
			case LuaValueType.Ptr:
				{
//					toPointer() -> serialization(NULL, encoder);
					break;
				}
			default:
				break;
			}
		}

		/// <summary>
		/// 获取值类型
		/// </summary>
		/// <value>类型</value>
		public LuaValueType type
		{
			get 
			{
				return _type;
			}
		}	

		/// <summary>
		/// 初始化LuaValue对象
		/// </summary>
		/// <param name="decoder">对象解码器</param>
		public LuaValue (LuaObjectDecoder decoder)
		{
			_type = (LuaValueType)decoder.readInt16 ();
			_value = null;

			switch (_type) 
			{
			case LuaValueType.Integer:
				_value = decoder.readInt32 ();
				break;
			case LuaValueType.Boolean:
				_value = decoder.readByte ();
				break;
			case LuaValueType.Number:
				_value = decoder.readDouble ();
				break;
			case LuaValueType.Data:
				_value = decoder.readBytes ();
				break;
			case LuaValueType.String:
				_value = decoder.readString ();
				break;
			case LuaValueType.Array:
				_value = readArrayList (decoder);
				break;
			case LuaValueType.Map:
				_value = readHashtable (decoder);
				break;
			}
		}

		/// <summary>
		/// 转换为整型
		/// </summary>
		/// <returns>整型值</returns>
		public int toInteger()
		{
			return Convert.ToInt32 (_value);
		}

		/// <summary>
		/// 转换为布尔值
		/// </summary>
		/// <returns>布尔值</returns>
		public bool toBoolean()
		{
			return Convert.ToBoolean (_value);
		}

		/// <summary>
		/// 转换为双精度浮点型
		/// </summary>
		/// <returns>双精度浮点值</returns>
		public double toNumber()
		{
			return Convert.ToDouble (_value);
		}

		/// <summary>
		/// 转换为二进制数据流
		/// </summary>
		/// <returns>二进制数据流</returns>
		public byte[] toData()
		{
			return (byte[])_value;
		}

		/// <summary>
		/// 转换为UTF8编码字符串
		/// </summary>
		/// <returns>字符串</returns>
		public string toString()
		{
			return Convert.ToString (_value);
		}

		/// <summary>
		/// 转换为数组
		/// </summary>
		/// <returns>数组对象</returns>
		public List<LuaValue> toArray()
		{
			return _value as List<LuaValue>;
		}

		/// <summary>
		/// 转换为字典
		/// </summary>
		/// <returns>字典对象</returns>
		public Dictionary<string, LuaValue> toMap()
		{
			return _value as Dictionary<string, LuaValue>;
		}
	}
}