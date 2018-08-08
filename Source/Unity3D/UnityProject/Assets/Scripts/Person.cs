using UnityEngine;
using System.Collections;
using System.CodeDom;
using cn.vimfung.luascriptcore;
using System;

public class Person : LuaExportType 
{
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

	public static Person createPersonError()
	{
		try
		{
			LuaContext.currentContext.raiseException ("can't create person");
			return new Person ();
		}
		catch (Exception ex)
		{
			return null;
		}
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
