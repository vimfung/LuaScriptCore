using System;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 上下文配置信息
	/// </summary>
	public class LuaContextConfig
	{
		/// <summary>
		/// 默认配置
		/// </summary>
		private static LuaContextConfig _config = new LuaContextConfig();

		/// <summary>
		/// 获取默认配置信息
		/// </summary>
		/// <value>默认配置信息.</value>
		public static LuaContextConfig defaultConfig
		{
			get
			{
				return _config;
			}
		}

		/// <summary>
		/// 手动导入类型开关，设置为YES时，在lua中需要使用某个原生类型时需要调用nativeType('typeName')方法来导入指定类型。默认为NO。
		/// 设置该开关目的是为了提升执行的效率，因为一般情况下并非是所有导出类都需要一开始就进行加载，如果导出类型很多的情况就会导致启动变慢。
		/// 使用该开关可以合理分配加载类型的时机。
		/// </summary>
		public bool manualImportClassEnabled;
	}
}

