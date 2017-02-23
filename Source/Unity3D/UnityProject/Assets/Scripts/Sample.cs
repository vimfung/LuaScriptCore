using UnityEngine;
using System.Collections;
using cn.vimfung.luascriptcore;
using UnityEngine.SceneManagement;
using System.Collections.Generic;
using cn.vimfung.luascriptcore.modules.oo;
using System.CodeDom;

public class Sample : MonoBehaviour {

	/// <summary>
	/// 是否注册方法
	/// </summary>
	private bool _isRegMethod = false;

	/// <summary>
	/// 是否注册模块
	/// </summary>
	private bool _isRegModule = false;

	/// <summary>
	/// 是否注册类型
	/// </summary>
	private bool _isRegClass = false;

	public void Start()
	{
		LuaContext.currentContext.onException((errMessage) => {

			Debug.Log(errMessage);

		});
	}

	/// <summary>
	/// 解析脚本按钮点击
	/// </summary>
	public void evalScriptButtonClickedHandler ()
	{
		LuaValue retValue = LuaContext.currentContext.evalScript ("print(10);return 'Hello World';");
		Debug.Log (string.Format("{0}", retValue.toString()));
	}

	/// <summary>
	/// 注册方法按钮点击
	/// </summary>
	public void registerMethodButtonClickedHandler()
	{
		if (!_isRegMethod) 
		{
			LuaContext.currentContext.registerMethod("getDeviceInfo", (arguments) => {

				Dictionary<string, LuaValue> info = new Dictionary<string, LuaValue>();
				info.Add("productName", new LuaValue(Application.productName));
				return new LuaValue(info);
			});

			_isRegMethod = true;
		}

		LuaContext.currentContext.evalScriptFromFile (string.Format("{0}/main.lua", Application.streamingAssetsPath));
	}

	/// <summary>
	/// 调用lua方法按钮点击
	/// </summary>
	public void callLuaMethodButtonClickedHandler()
	{
		//加载Lua脚本
		LuaContext.currentContext.evalScriptFromFile("todo.lua");
		LuaValue retValue = LuaContext.currentContext.callMethod ("add", new List<LuaValue> (){ new LuaValue (1000.0), new LuaValue (24.0)});
		Debug.Log (string.Format ("result = {0}", retValue.toNumber ()));
	}

	/// <summary>
	/// 注册模块方法按钮点击
	/// </summary>
	public void registerModuleButtonClickedHandler()
	{
		if (!_isRegModule)
		{
			_isRegModule = true;
			LuaContext.currentContext.registerModule<LogModule> ();
		}

		LuaContext.currentContext.evalScript ("LogModule.writeLog('Hello World!'); local a = LogModule.test({1,2,3,4}); print(a);");
	}

	/// <summary>
	/// 注册类型按钮点击
	/// </summary>
	public void registerClassButtonClickedHandler()
	{
		if (!_isRegClass)
		{
			_isRegClass = true;
			LuaContext.currentContext.registerModule<Person> ();
		}

		LuaContext.currentContext.evalScript ("local p = Person.createPerson(); print(p); p:setName('xxxx'); p:speak(); print(Person.printPerson(p));");
	}
}
