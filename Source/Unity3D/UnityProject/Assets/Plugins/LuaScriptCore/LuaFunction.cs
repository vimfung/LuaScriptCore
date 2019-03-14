using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// Lua方法
	/// </summary>
	public class LuaFunction : LuaBaseObject
	{
		private LuaContext _context;

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="decoder">对象解码器</param>
		public LuaFunction (LuaObjectDecoder decoder)
			: base(decoder)
		{
			int contextId = decoder.readInt32 ();
			_context = LuaContext.getContext (contextId);
			luaObjectId = decoder.readString ();
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization(LuaObjectEncoder encoder)
		{
			base.serialization (encoder);

			encoder.writeInt32 (_context.objectId);
			encoder.writeString (luaObjectId);
		}

		/// <summary>
		/// 调用
		/// </summary>
		/// <param name="arguments">Arguments.</param>
		public LuaValue invoke(List<LuaValue> arguments)
		{
			return invoke (arguments, null);
		}

		/// <summary>
		/// 调用方法
		/// </summary>
		/// <param name="arguments">参数列表</param>
		/// <param name="scriptController">脚本控制器</param>
		public LuaValue invoke(List<LuaValue> arguments, LuaScriptController scriptController)
		{
			int scriptControllerId = 0;
			if (scriptController != null)
			{
				scriptControllerId = scriptController.objectId;
			}

			IntPtr funcPtr = IntPtr.Zero;
			IntPtr argsPtr = IntPtr.Zero;
			IntPtr resultPtr = IntPtr.Zero;

			LuaObjectEncoder funcEncoder = new LuaObjectEncoder (_context);
			funcEncoder.writeObject (this);

			byte[] bytes = funcEncoder.bytes;
			funcPtr = Marshal.AllocHGlobal (bytes.Length);
			Marshal.Copy (bytes, 0, funcPtr, bytes.Length);

			if (arguments != null)
			{
				LuaObjectEncoder argEncoder = new LuaObjectEncoder (_context);
				argEncoder.writeInt32 (arguments.Count);
				foreach (LuaValue value in arguments)
				{
					argEncoder.writeObject (value);
				}

				bytes = argEncoder.bytes;
				argsPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, argsPtr, bytes.Length);
			}

			int size = NativeUtils.invokeLuaFunction (_context.objectId, funcPtr, argsPtr, scriptControllerId, out resultPtr);

			if (argsPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (argsPtr);
			}
			if (funcPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (funcPtr);
			}

			if (size > 0)
			{
				return LuaObjectDecoder.DecodeObject (resultPtr, size, _context) as LuaValue;
			}

			return new LuaValue ();
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
	}
}

