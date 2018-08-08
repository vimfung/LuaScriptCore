using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore;
using System;
using System.Collections.Generic;

public class LogModule : LuaExportType
{
	[LuaExclude]
	public static void writeLog(String message)
	{
		Debug.LogFormat ("string log = {0}", message);
	}
		
	public static void writeLog(int value)
	{
		Debug.LogFormat ("int log = {0}", value);
	}

	public static double test(UInt64[] a)
	{
		Debug.Log (a.ToString ());
	
		return 1024.05;
	}
}
