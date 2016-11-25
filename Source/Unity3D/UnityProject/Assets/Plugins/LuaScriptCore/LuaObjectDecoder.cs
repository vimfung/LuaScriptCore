using System.Collections;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace cn.vimfung.luascriptcore
{
	public class LuaObjectDecoder 
	{
		private byte[] _buffer;
		private int _offset;

		/// <summary>
		/// 初始化对象解码器
		/// </summary>
		/// <param name="objectPtr">原生层中缓冲区指针</param>
		/// <param name="size">缓冲区大小</param>
		public LuaObjectDecoder(IntPtr objectPtr, int size)
		{
			_offset = 0;
			_buffer = new byte[size];
			Marshal.Copy (objectPtr, _buffer, 0, size);
			Marshal.FreeHGlobal (objectPtr);
		}

		/// <summary>
		/// 读取一个16位的整型值
		/// </summary>
		/// <returns>整型值</returns>
		public Int16 readInt16()
		{
			Int16 value = Convert.ToInt16((_buffer [_offset] << 8) 
				| _buffer [_offset + 1]);
			_offset += 2;

			return value;
		}

		/// <summary>
		/// 读取一个32位的整型值
		/// </summary>
		/// <returns>整型值</returns>
		public Int32 readInt32()
		{
			Int32 value = (_buffer [_offset] << 24) 
				| (_buffer [_offset + 1] << 16) 
				| (_buffer [_offset + 2] << 8) 
				| _buffer [_offset + 3];
			_offset += 4;

			return value;
		}

		/// <summary>
		/// 读取一个64位的整型值
		/// </summary>
		/// <returns>整型值</returns>
		public Int64 readInt64()
		{
			Int64 value = (_buffer [_offset] << 56) 
				| (_buffer [_offset + 1] << 48) 
				| (_buffer [_offset + 2] << 40) 
				| (_buffer [_offset + 3] << 32)
				| (_buffer [_offset + 4] << 24) 
				| (_buffer [_offset + 5] << 16) 
				| (_buffer [_offset + 6] << 8) 
				| _buffer [_offset + 7];
			_offset += 8;

			return value;
		}

		/// <summary>
		/// 读取一个双精度浮点型数据
		/// </summary>
		/// <returns>浮点型数值</returns>
		public double readDouble()
		{
			double value = BitConverter.ToDouble (_buffer, _offset);
			_offset += 8;

			return value;
		}

		/// <summary>
		/// 读取一个字符串
		/// </summary>
		/// <returns>字符串.</returns>
		public string readString()
		{
			int len = readInt32 ();

			byte[] stringBytes = new byte[len];
			Buffer.BlockCopy (_buffer, _offset, stringBytes, 0, len);
			_offset += len;

			return Encoding.UTF8.GetString (stringBytes);
		}

		/// <summary>
		/// 读取一个字节
		/// </summary>
		/// <returns>字节值</returns>
		public Byte readByte()
		{
			Byte value = _buffer [_offset];
			_offset++;

			return value;
		}

		/// <summary>
		/// 读取一段缓冲区数据
		/// </summary>
		/// <returns>二进制数组.</returns>
		public byte[] readBytes()
		{
			int len = readInt32 ();

			byte[] bytes = new byte[len];
			Buffer.BlockCopy (_buffer, _offset, bytes, 0, len);
			_offset += len;

			return bytes;
		}

		/// <summary>
		/// 读取一个对象类型
		/// </summary>
		/// <returns>对象</returns>
		public object readObject()
		{
			if (readByte () == 'L')
			{
				string className = readString ();
				if (readByte () == ';') 
				{
					//反射对象
					Type t = Type.GetType (className);
					if (t != null) 
					{
						object[] parameters = new object[1];
						parameters [0] = this;

						return Activator.CreateInstance (t, parameters);
					}
				}
			}

			return null;
		}

		/// <summary>
		/// 解码对象
		/// </summary>
		/// <returns>对象</returns>
		/// <param name="objectPtr">表示C中的二进制数据缓冲区指针</param>
		/// <param name="size">缓冲区大小</param>
		public static object DecodeObject(IntPtr objectPtr, int size)
		{
			LuaObjectDecoder decoder = new LuaObjectDecoder (objectPtr, size);
			return decoder.readObject ();
		}
	}
}