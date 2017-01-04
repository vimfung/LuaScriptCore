using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore;
using System;
using System.Collections.Generic;

public class LogModule : LuaModule
{
	public static void writeLog(String message)
	{
		Debug.Log(message);
	}

	public static double test(UInt64[] a)
	{
		Debug.Log (a.ToString ());
	
		return 1024.05;
	}
}
