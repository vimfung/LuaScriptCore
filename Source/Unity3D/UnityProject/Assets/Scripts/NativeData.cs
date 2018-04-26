using System;
using cn.vimfung.luascriptcore;

[LuaExportTypeAnnotation()]
public class NativeData : LuaExportType
{
	public NativeData ()
	{
	}

	private string _dataId;

	public string dataId
	{
		get
		{
			return _dataId;
		}
		set
		{
			_dataId = value;
		}
	}

	public static Person createPerson()
	{
		return new Person ();
	}
}

