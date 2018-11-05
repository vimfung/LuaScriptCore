using System.Collections;
using System.Collections.Generic;
using System;
using System.Text;
using System.Runtime.InteropServices;

namespace cn.vimfung.luascriptcore
{
	public class LuaObjectEncoder 
	{
		private List<byte> _buffer;
		private LuaContext _context;

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="context">上下文对象.</param>
		public LuaObjectEncoder (LuaContext context)
		{
			_buffer = new List<byte> ();
			_context = context;
		}

		/// <summary>
		/// 获取上下文对象
		/// </summary>
		/// <value>上下文对象.</value>
		public LuaContext context
		{
			get
			{
				return _context;
			}
		}

		/// <summary>
		/// 写入一个16位整型
		/// </summary>
		/// <param name="value">16位整型值</param>
		public void writeInt16(Int16 value)
		{
			byte[] bytes = BitConverter.GetBytes (value);
			if (BitConverter.IsLittleEndian)
			{
				Array.Reverse (bytes);
			}
			_buffer.AddRange (bytes);
		}

		/// <summary>
		/// 写入一个32位整型
		/// </summary>
		/// <param name="value">32位整型值</param>
		public void writeInt32(Int32 value)
		{
			byte[] bytes = BitConverter.GetBytes ((int)value);
			if (BitConverter.IsLittleEndian)
			{
				Array.Reverse (bytes);
			}
			_buffer.AddRange (bytes);
		}

		/// <summary>
		/// 写入一个64位的整型值
		/// </summary>
		/// <param name="value">64位整型值</param>
		public void writeInt64(Int64 value)
		{
			byte[] bytes = BitConverter.GetBytes (value);
			if (BitConverter.IsLittleEndian)
			{
				Array.Reverse (bytes);
			}
			_buffer.AddRange (bytes);
		}

		/// <summary>
		/// 写入一个双精度浮点型数据
		/// </summary>
		/// <param name="value">双精度浮点型</param>
		public void writeDouble(double value)
		{
			byte[] bytes = BitConverter.GetBytes (value);
			if (!BitConverter.IsLittleEndian)
			{
				//置换数组位置
				Array.Reverse (bytes);
			}
			_buffer.AddRange (bytes);
		}

		/// <summary>
		/// 写入一个字符串
		/// </summary>
		/// <param name="value">字符串</param>
		public void writeString(string value)
		{
			if (value != null)
			{
				byte[] buf = System.Text.Encoding.UTF8.GetBytes (value);
				writeInt32 (buf.Length);
				_buffer.AddRange (buf);
			}
			else
			{
				writeInt32 (0);
			}
		}

		/// <summary>
		/// 写入一个字节
		/// </summary>
		/// <param name="value">字符串</param>
		public void writeByte(Byte value)
		{
			_buffer.Add (value);
		}

		/// <summary>
		/// 写入一段缓冲区数据
		/// </summary>
		/// <param name="value">缓冲区数据</param>
		public void writeBytes(byte[] value)
		{
			writeInt32 (value.Length);
			_buffer.AddRange (value);
		}

		/// <summary>
		/// 写入一个对象类型
		/// </summary>
		/// <param name="value">对象</param>
		public void writeObject(object value)
		{
			if (value != null)
			{
				if (value is LuaBaseObject)
				{
					this.writeByte ((byte)'L');
					this.writeString (value.GetType ().Name);
					this.writeByte ((byte)';');

					(value as LuaBaseObject).serialization (this);
				}
				else
				{
					LuaObjectReference objRef = new LuaObjectReference (value);
					writeInt64 (objRef.referenceId);
				}
			}
		}

		/// <summary>
		/// 获取二进制数组
		/// </summary>
		/// <value>二进制数组</value>
		public byte[] bytes
		{
			get
			{
				return _buffer.ToArray();
			}
		}
	}
}
