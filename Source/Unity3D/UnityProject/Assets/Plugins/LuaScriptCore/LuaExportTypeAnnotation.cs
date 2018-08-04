using System;

namespace cn.vimfung.luascriptcore
{
	[Obsolete("Use LuaExclude attribute instead.")]
	[AttributeUsage(AttributeTargets.Class)]
	public class LuaExportTypeAnnotation : Attribute
	{

		/// <summary>
		/// 获取／设置排除导出导出类方法名称列表
		/// </summary>
		/// <value>排除导出导出类方法名称列表.</value>
		public string[] excludeExportClassMethodNames
		{
			get;
			set;
		}

		/// <summary>
		/// 获取／设置排除导出实例方法名称列表
		/// </summary>
		/// <value>排除导出实例方法名称列表.</value>
		public string[] excludeExportInstanceMethodNames
		{
			get;
			set;
		}

		/// <summary>
		/// 获取／设置排除导出属性名称列表
		/// </summary>
		/// <value>排除导出属性名称列表.</value>
		public string[] excludeExportPropertyNames
		{
			get;
			set;
		}
	}
}

