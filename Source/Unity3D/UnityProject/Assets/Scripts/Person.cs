using UnityEngine;
using System.Collections;
using System.CodeDom;
using cn.vimfung.luascriptcore;

public class Person : LuaExportType 
{
	internal static string typeName = "Person";
	internal static string[] excludeExportClassMethodNames = new string[] { };
	internal static string[] excludeExportInstanceMethodsNames = new string[] { };
	internal static string[] excludeExportPropertyNames = new string[] { };

	private string _name;
	public string name
	{
		get 
		{
			return _name;
		}
		set
		{
			_name = value;
		}
	}

	public static Person printPerson(Person p)
	{
		Debug.Log (p.name);
		return p;
	}

	public static Person createPerson()
	{
		return new Person ();
	}

	public void speak()
	{
		Debug.Log (string.Format("{0} speak", this.name));
	}

	private static LuaValue _func;

	public static void retainHandler(LuaFunction handler)
	{
		_func = new LuaValue (handler);
		LuaContext.currentContext.retainValue (_func);
	}

	public static void releaseHandler()
	{
		LuaContext.currentContext.releaseValue (_func);
	}

	public static void callHandler()
	{
		if (_func != null)
		{
			_func.toFunction ().invoke (null);
		}
	}
}
