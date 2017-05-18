using System;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// Lua对象接口，需要返回给Lua层的对象可以使用该协议转换为Lua层能识别的对象
	/// </summary>
	public interface ILuaObject
	{
		LuaObjectDescriptor getDescriptor();
	}
}

