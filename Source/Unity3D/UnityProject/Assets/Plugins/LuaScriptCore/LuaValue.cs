using System.Collections;
using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using UnityEngine;

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
		Function = 10,
		Tuple = 11
	};

	public class LuaValue : LuaBaseObject
	{
		private LuaValueType _type;
		private object _value;
		private string _tableId;
		private LuaContext _context;

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
		/// 初始化一个方法值
		/// </summary>
		/// <param name="value">Lua方法.</param>
		public LuaValue (LuaFunction value)
		{
			_value = value;
			_type = LuaValueType.Function;
		}

		/// <summary>
		/// 初始化一个指针值
		/// </summary>
		/// <param name="value">Lua指针.</param>
		public LuaValue (LuaPointer value)
		{
			_value = value;
			_type = LuaValueType.Ptr;
		}

		/// <summary>
		/// 初始化一个元组值
		/// </summary>
		/// <param name="value">元组.</param>
		public LuaValue (LuaTuple value)
		{
			_value = value;
			_type = LuaValueType.Tuple;
		}

		/// <summary>
		/// 初始化一个对象值
		/// </summary>
		/// <param name="value">对象</param>
		public LuaValue (object value)
		{
			if (value != null)
			{
				
				if (value is int
				    || value is uint
				    || value is Int16
				    || value is UInt16
				    || value is Int64
				    || value is UInt64)
				{
					_value = value;
					_type = LuaValueType.Integer;
				}
				else if (value is double
				         || value is float)
				{
					_value = value;
					_type = LuaValueType.Number;
				}
				else if (value is bool)
				{
					_value = value;
					_type = LuaValueType.Boolean;
				}
				else if (value is byte[])
				{
					_value = value;
					_type = LuaValueType.Data;
				}
				else if (value is string)
				{
					_value = value;
					_type = LuaValueType.String;
				}
				else if (value is Array)
				{
					List<LuaValue> arr = new List<LuaValue> ();
					_value = arr;
					_type = LuaValueType.Array;

					//转换数据
					foreach (object item in (value as Array))
					{
						LuaValue itemValue = new LuaValue (item);
						arr.Add (itemValue);
					}
				}
				else if (value is IDictionary)
				{
					Dictionary<string, LuaValue> dict = new Dictionary<string, LuaValue> ();
					_value = dict;
					_type = LuaValueType.Map;

					foreach (DictionaryEntry de in (value as IDictionary))
					{
						LuaValue itemValue = new LuaValue (de.Value);
						dict.Add (Convert.ToString (de.Key), itemValue);
					}
				}
				else if (value is LuaFunction)
				{
					_value = value;
					_type = LuaValueType.Function;
				}
				else if (value is LuaPointer)
				{
					_value = value;
					_type = LuaValueType.Ptr;
				}
				else if (value is LuaTuple)
				{
					_value = value;
					_type = LuaValueType.Tuple;
				}
				else
				{
					if (value is LuaObjectDescriptor)
					{
						_value = value;
					}
					else if (value is ILuaObject)
					{
						_value = (value as ILuaObject).getDescriptor ();
					}
					else
					{
						_value = new LuaObjectDescriptor (value);
					}
					_type = LuaValueType.Object;
				}
			}
			else
			{
				_value = null;
				_type = LuaValueType.Nil;
			}
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

			if (_context != null)
			{
				encoder.writeInt32 (_context.objectId);
			}
			else
			{
				encoder.writeInt32 (0);
			}

			encoder.writeString (_tableId);
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
						encoder.writeObject (value);
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
					encoder.writeObject (_value);
					break;
				}
			case LuaValueType.Boolean:
				{
					encoder.writeByte (Convert.ToByte(toBoolean ()));
					break;
				}
			case LuaValueType.Function:
				{
					encoder.writeObject (toFunction());
					break;
				}
			case LuaValueType.Ptr:
				{
					encoder.writeObject (toPointer());
					break;
				}
			case LuaValueType.Tuple:
				{
					encoder.writeObject (toTuple());
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
			: base(decoder)
		{

			int contextId = decoder.readInt32 ();
			_context = LuaContext.getContext (contextId);
			_tableId = decoder.readString ();
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
			case LuaValueType.Function:
				_value = decoder.readObject () as LuaFunction;
				break;
			case LuaValueType.Ptr:
				_value = decoder.readObject () as LuaPointer;
				break;
			case LuaValueType.Tuple:
				_value = decoder.readObject () as LuaTuple;
				break;
			case LuaValueType.Object:
				_value = decoder.readObject ();
				break;
			}
		}

		/// <summary>
		/// 转换为整型
		/// </summary>
		/// <returns>整型值</returns>
		public int toInteger()
		{
			try
			{
				return Convert.ToInt32 (_value);
			}
			catch(FormatException ex)
			{
				Debug.LogFormat ("{0}", ex.Message);
				return 0;
			}
		}

		/// <summary>
		/// 转换为布尔值
		/// </summary>
		/// <returns>布尔值</returns>
		public bool toBoolean()
		{
			try
			{
				return Convert.ToBoolean (_value);
			}
			catch (FormatException ex)
			{
				Debug.LogFormat ("{0}", ex.Message);
				return false;
			}
		}

		/// <summary>
		/// 转换为双精度浮点型
		/// </summary>
		/// <returns>双精度浮点值</returns>
		public double toNumber()
		{
			try
			{
				return Convert.ToDouble (_value);
			}
			catch (FormatException ex)
			{
				Debug.LogFormat ("{0}", ex.Message);
				return 0.0;
			}

		}

		/// <summary>
		/// 转换为二进制数据流
		/// </summary>
		/// <returns>二进制数据流</returns>
		public byte[] toData()
		{
			try
			{
				switch (_type) 
				{
				case LuaValueType.Integer:
					return BitConverter.GetBytes (Convert.ToInt32 (_value));
				case LuaValueType.Number:
					return BitConverter.GetBytes (Convert.ToDouble(_value));
				case LuaValueType.String:
					return System.Text.Encoding.UTF8.GetBytes (Convert.ToString(_value));
				case LuaValueType.Data:
					return (byte[])_value;
				default:
					return null;
				}
			}
			catch (FormatException ex)
			{
				Debug.LogFormat ("{0}", ex.Message);
				return null;
			}
		}

		/// <summary>
		/// 转换为UTF8编码字符串
		/// </summary>
		/// <returns>字符串</returns>
		public string toString()
		{
			try
			{
				return Convert.ToString (_value);
			}
			catch (FormatException ex)
			{
				Debug.LogFormat ("{0}", ex.Message);
				return null;
			}
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

		/// <summary>
		/// 转换为Lua方法
		/// </summary>
		/// <returns>Lua方法.</returns>
		public LuaFunction toFunction()
		{
			return _value as LuaFunction;
		}

		/// <summary>
		/// 转换为Lua指针
		/// </summary>
		/// <returns>Lua指针.</returns>
		public LuaPointer toPointer()
		{
			return _value as LuaPointer;
		}

		/// <summary>
		/// 转换为元组
		/// </summary>
		/// <returns>元组.</returns>
		public LuaTuple toTuple()
		{
			return _value as LuaTuple;
		}

		/// <summary>
		/// 转换为对象
		/// </summary>
		/// <returns>对象.</returns>
		public object toObject()
		{
			//解包数组和字典中的元素
			if (this.type == LuaValueType.Map)
			{
				Dictionary<string, LuaValue> map = toMap ();
				if (map != null)
				{
					Hashtable dict = new Hashtable ();
					foreach (KeyValuePair<string, LuaValue> kv in map)
					{
						dict.Add (kv.Key, kv.Value.toObject ());
					}
					return dict;
				}
			}
			else if (this.type == LuaValueType.Array)
			{
				List<LuaValue> arr = toArray();
				if (arr != null)
				{
					ArrayList objArr = new ArrayList ();
					foreach (LuaValue item in arr) 
					{
						objArr.Add (item.toObject ());
					}
					return objArr.ToArray ();
				}
			}

			if (_value is LuaObjectDescriptor)
			{
				return (_value as LuaObjectDescriptor).obj;
			}

			return _value;
		}

		/// <summary>
		/// 设置对象
		/// </summary>
		/// <param name="keyPath">键名路径.</param>
		/// <param name="value">值.</param>
		public void setObject(String keyPath, object value)
		{
			if (_context != null && _tableId != null && type == LuaValueType.Map)
			{
				LuaValue objectValue = new LuaValue (value);

				IntPtr resultPtr = IntPtr.Zero;

				IntPtr valuePtr = IntPtr.Zero;
				LuaObjectEncoder encoder = new LuaObjectEncoder (_context);
				encoder.writeObject (this);
				byte[] valueBytes = encoder.bytes;
				valuePtr = Marshal.AllocHGlobal (valueBytes.Length);
				Marshal.Copy (valueBytes, 0, valuePtr, valueBytes.Length);


				IntPtr objPtr = IntPtr.Zero;
				LuaObjectEncoder objEncoder = new LuaObjectEncoder (_context);
				objEncoder.writeObject (objectValue);

				byte[] bytes = objEncoder.bytes;
				objPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, objPtr, bytes.Length);

				int bufferLen = NativeUtils.tableSetObject (_context.objectId, valuePtr, keyPath, objPtr, resultPtr);

				if (bufferLen > 0)
				{
					LuaValue resultValue = LuaObjectDecoder.DecodeObject (resultPtr, bufferLen, _context) as LuaValue;
					_value = resultValue._value;
				}

				if (valuePtr != IntPtr.Zero)
				{
					Marshal.FreeHGlobal (valuePtr);
				}

				if (objPtr != IntPtr.Zero)
				{
					Marshal.FreeHGlobal (objPtr);
				}
			}
		}
	}
}