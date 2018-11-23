using System;

namespace cn.vimfung.luascriptcore
{
	public class LuaManagedValue : LuaBaseObject
	{
		private LuaValue _source;
		private LuaContext _context;

		public LuaManagedValue (LuaValue value, LuaContext context)
		{
			_source = value;
			_context = context;

			_context.retainValue (_source);
		}

		~LuaManagedValue ()
		{
			if (_context != null && _source != null)
			{
				_context.releaseValue (_source);
			}
		}

		/// <summary>
		/// 获取源数据
		/// </summary>
		/// <value>数据对象.</value>
		public LuaValue source
		{
			get
			{
				return _source;
			}
		}
	}
}

