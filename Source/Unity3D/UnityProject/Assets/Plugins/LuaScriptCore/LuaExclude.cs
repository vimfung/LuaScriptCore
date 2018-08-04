using System;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 用于排除C#类型导出到Lua的属性、方法
	/// </summary>
	[AttributeUsage(AttributeTargets.All)]
	public class LuaExclude : Attribute
	{
		
	}
}

