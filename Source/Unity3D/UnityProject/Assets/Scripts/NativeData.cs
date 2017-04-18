using System;


public class NativeData
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

