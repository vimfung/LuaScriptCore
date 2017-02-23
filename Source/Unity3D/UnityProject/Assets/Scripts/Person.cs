using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore.modules.oo;
using System.CodeDom;

public class Person : LuaObjectClass 
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

	public void speak()
	{
		Debug.Log (string.Format("{0} speak", this.name));
	}
}
