using System;
using System.Reflection;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 导出类型配置
	/// </summary>
	internal sealed class LuaExportTypeConfig
	{
		/// <summary>
		/// 获取导出类型配置
		/// </summary>
		/// <returns>类型配置.</returns>
		/// <param name="t">类型.</param>
		internal static LuaExportTypeConfig Create(Type t)
		{
			return new LuaExportTypeConfig (t);
		}

		private string _typeName = null;
		private string[] _excludeExportClassMethodNames = null;
		private string[] _excludeExportInstanceMethodNames = null;
		private string[] _excludeExportPropertyNames = null;

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="t">类型.</param>
		private LuaExportTypeConfig(Type t)
		{
			_typeName = t.Name;

			FieldInfo[] fields = t.GetFields (BindingFlags.Static | BindingFlags.NonPublic);
			foreach (FieldInfo field in fields)
			{
				switch (field.Name)
				{
				case "typeName":
					_typeName = field.GetValue (t) as string;
					break;
				case "excludeExportClassMethodNames":
					_excludeExportClassMethodNames = field.GetValue (t) as string[];
					break;
				case "excludeExportInstanceMethodsNames":
					_excludeExportInstanceMethodNames = field.GetValue (t) as string[];
					break;
				case "excludeExportPropertyNames":
					_excludeExportPropertyNames = field.GetValue (t) as string[];
					break;
				}
			}
		}

		/// <summary>
		/// 获取类型名称
		/// </summary>
		/// <value>类型名称.</value>
		internal string typeName
		{
			get
			{
				return _typeName;
			}
		}

		/// <summary>
		/// 获取排除导出类型方法名称列表
		/// </summary>
		/// <value>排除导出类型方法名称列表.</value>
		internal string[] excludeExportClassMethodNames
		{
			get
			{
				return _excludeExportClassMethodNames;
			}
		}

		/// <summary>
		/// 获取排除导出实例方法名称列表
		/// </summary>
		/// <value>排除导出实例方法名称列表.</value>
		internal string[] excludeExportInstanceMethodNames
		{
			get
			{
				return _excludeExportInstanceMethodNames;
			}
		}

		/// <summary>
		/// 获取排除导出属性名称列表
		/// </summary>
		/// <value>T排除导出属性名称列表.</value>
		internal string[] excludeExportPropertyNames
		{
			get
			{
				return _excludeExportPropertyNames;
			}
		}
	}
}

